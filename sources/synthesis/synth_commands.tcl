create_project vicuna -force -part xc7k325tffg900-2

#set RTL_DIR $env(RTL_DIR)
#set SYNTH_DIR $env(SYNTH_DIR)

set RTL_DIR /home/parker/Desktop/vicuna_edu_setup/vicuna_edu/sources/rtl/
set SYNTH_DIR /home/parker/Desktop/vicuna_edu_setup/vicuna_edu/sources/synthesis/

#Add sources.  Order matters

#CV32E40X Sources
read_verilog [ glob $RTL_DIR/cv32e40x/rtl/include/*.sv ]
read_verilog [ glob $SYNTH_DIR/cv32e40x_sim_clock_gate.sv ]
read_verilog [ glob $RTL_DIR/cv32e40x/rtl/*.sv ]

read_verilog $RTL_DIR/vicuna2_core/cvfpu/src/fpnew_pkg.sv

#Vicuna2 Sources
read_verilog $RTL_DIR/vicuna2_core/rtl/vproc_pkg.sv
#vproc config must be generated first by existing CMake project
read_verilog $RTL_DIR/vicuna2_core/vproc_config.sv

read_verilog [ glob $RTL_DIR/vicuna2_core/rtl/*.sv ]
read_verilog [ glob $RTL_DIR/vicuna2_core/sva/*.svh ]


#Top Sources
read_verilog [ glob $RTL_DIR/*.sv ]


#Difficulties with variable -verilog_define statements/tcl parsing.  A dumb solution that works because of dependencies between ISAs

synth_design -part xc7k325tffg900-2 -top vproc_top -include_dirs { $RTL_DIR/vicuna2_core/cvfpu/src/common_cells/include/common_cells } -verilog_define XIF_ON -verilog_define RISCV_ZVE32X -verilog_define OLD_VICUNA -max_dsp 0


# Determine highest possible valid clock frequency, starting at 10Mhz
set iter 0
set clk_period 100
set prev_valid_period $clk_period
create_clock -name Clk -period $clk_period [get_ports clk_i]
#Set clock timing constraint before placement+routing
set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets clk_i]
#opt_design

#opt_design -resynth_remap

place_design


#set critPath [get_nets -of [get_timing_paths -max_paths 100]]
#route_design -nets [get_nets $critPath] -auto_delay

#phys_opt_design -critical_pin_opt
#phys_opt_design -clock_opt


route_design



set pass [expr {[get_property SLACK [get_timing_paths -delay_type min_max]] >= 0}]

if {!$pass} {
    report_timing
    report_methodology
    report_timing_summary -check_timing_verbose
    report_utilization
    puts "ERROR:Failed timing at initial clk_period : $clk_period ns.  Choose larger start point or solve violations unrelated to clock frequency."
    #start_gui
    exit

} else {
    report_timing
    report_timing_summary -check_timing_verbose
    report_utilization
    set freq [expr (1/(1e-9 * $prev_valid_period))/1e6]
    puts "After $iter iterations: Min Clk Period $prev_valid_period ns; $freq MHz"

    exit
}
#     set prev_valid_period $clk_period
#     set prev_failed_period 0.0
#     #clk periods need to be whole ps values.  This simple solution works
#     set clk_period [expr double(round(1000*$clk_period / 2))/1000]
#
#     for {set i 0} {$i < $iter} {incr i} {
#         set freq_last_succ [expr (1/(1e-9 * $prev_valid_period))/1e6]
#         set freq_last_fail [expr (1/(1e-9 * $prev_failed_period))/1e6]
#         set freq_next [expr (1/(1e-9 * $clk_period))/1e6]
#         set iplus1 [expr $i+1 ]
#         puts "###################################################"
#         puts "Iteration $iplus1 / $iter"
#         puts "Testing $clk_period ns | $freq_next MHz"
#         puts "Last Successful : $prev_valid_period ns | $freq_last_succ MHz"
#         puts "Last Failed     : $prev_failed_period ns | $freq_last_fail MHz"
#         puts "###################################################"
#         route_design -unroute
#         place_design -unplace
#
#         create_clock -name Clk -period $clk_period [get_ports clk_i] -quiet
#
#         opt_design
#         place_design
#         phys_opt_design
#         route_design
#         phys_opt_design
#
#         set pass [expr {[get_property SLACK [get_timing_paths -delay_type min_max]] >= 0}]
#
#         if {!$pass} {
#             puts "Failed Timing"
#             #selected frequency did not pass timing.  Pick midpoint between last failed and last passing
#             set prev_failed_period $clk_period
#             set clk_period [expr double(round(1000*($prev_failed_period + ($prev_valid_period - $prev_failed_period) / 2)))/1000]
#         } else {
#             #selected freqency passed timing.  Pick midpoint between last failed and last passing
#             puts "Passed Timing"
#             set prev_valid_period $clk_period
#             set clk_period [expr double(round(1000*($prev_failed_period + ($prev_valid_period - $prev_failed_period) / 2)))/1000]
#         }





