create_project vicuna -force -part xc7s100fgga676-1Q

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

if {[catch "synth_design -part xc7s100fgga676-1Q -top vproc_top -include_dirs { $RTL_DIR/vicuna2_core/cvfpu/src/common_cells/include/common_cells } -verilog_define XIF_ON -verilog_define RISCV_ZVE32X -verilog_define OLD_VICUNA -verilog_define SYNTH -max_dsp 0" errorstring]} {
  puts " Error - $errorstring "
  exit
}

# Determine highest possible valid clock frequency, starting at 10Mhz
set iter 0
set clk_period 100
set prev_valid_period $clk_period
create_clock -name Clk -period $clk_period [get_ports clk_i]
#Set clock timing constraint before placement+routing
set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets clk_i]

if {[catch "place_design" errorstring]} {
  puts " Error - $errorstring "
  exit
}

if {[catch "route_design" errorstring]} {
  puts " Error - $errorstring "
  exit
}

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






