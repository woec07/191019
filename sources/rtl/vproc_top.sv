// Copyright TU Wien
// Licensed under the Solderpad Hardware License v2.1, see LICENSE.txt for details
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


module vproc_top import vproc_pkg::*; #(
        parameter int unsigned     MEM_W         = vproc_config::MEM_W,  // memory bus width in bits
        parameter int unsigned     VMEM_W        = vproc_config::VMEM_W,  // vector memory interface width in bits
        parameter vreg_type        VREG_TYPE     = VREG_GENERIC,
        parameter mul_type         MUL_TYPE      = MUL_GENERIC
    )(
        input  logic               clk_i,
        input  logic               rst_ni,

        output logic               mem_req_o,
        output logic [31:0]        mem_addr_o,
        output logic               mem_we_o,
        output logic [MEM_W/8-1:0] mem_be_o,
        output logic [MEM_W  -1:0] mem_wdata_o,
        input  logic               mem_rvalid_i,
        input  logic               mem_err_i,
        input  logic [MEM_W  -1:0] mem_rdata_i,

        output logic               data_read_o,

        output logic [31:0]        pend_vreg_wr_map_o,

        output logic               mem_ireq_o,
        output logic [31:0]        mem_iaddr_o,
        input  logic               mem_irvalid_i,
        input  logic               mem_ierr_i,
        input  logic [32  -1:0]    mem_irdata_i,

        output logic               data_iread_o
              
    );

    if ((MEM_W & (MEM_W - 1)) != 0 || MEM_W < 32) begin
        $fatal(1, "The memory bus width MEM_W must be at least 32 and a power of two.  ",
                  "The current value of %d is invalid.", MEM_W);
    end

    if ((vproc_config::PIPE_CNT) <= 1) begin
        $fatal(1, "Vicuna Requires at least two vector pipelines.");
    end

    if ((vproc_config::VREG_W & (vproc_config::VREG_W - 1)) != 0 || vproc_config::VREG_W < 64) begin
        $fatal(1, "The Vector Registers (VLEN) must be at least 64 and a power of two.  ",
                  "The current value of %d is invalid.", vproc_config::VREG_W);
    end

    // Reset synchronizer (sync reset is used for Vicuna by default, async reset for the core)
    logic [3:0] rst_sync_qn;
    logic sync_rst_n;
    always_ff @(posedge clk_i) begin
        rst_sync_qn[0] <= rst_ni;
        for (int i = 1; i < 4; i++) begin
            rst_sync_qn[i] <= rst_sync_qn[i-1];
        end
    end
    assign sync_rst_n = rst_sync_qn[3];

    ///////////////////////////////////////////////////////////////////////////
    // MAIN CORE INTEGRATION

    // Instruction fetch interface
    logic        instr_req;
    logic [31:0] instr_addr;
    logic        instr_gnt;
    logic        instr_rvalid;
    logic        instr_err;
    logic [31:0] instr_rdata;

    // Data load & store interface
    logic        sdata_req;
    logic [31:0] sdata_addr;
    logic        sdata_we;
    logic  [3:0] sdata_be;
    logic [31:0] sdata_wdata;
    logic        sdata_gnt;
    logic        sdata_rvalid;
    logic        sdata_err;
    logic [31:0] sdata_rdata;

   

    ///////////////////////////Top level xif interface, some signals used for the memory units

    // Vector Unit Interface
    localparam X_NUM_RS = 3;
    localparam X_ID_WIDTH = 4;
    localparam X_RFR_WIDTH = 32;
    localparam X_RFW_WIDTH = 32;
    localparam X_MISA = 0;
    vproc_xif #(
        .X_NUM_RS    ( X_NUM_RS    ),
        .X_ID_WIDTH  ( X_ID_WIDTH  ),
        .X_MEM_WIDTH ( VMEM_W      ),
        .X_RFR_WIDTH ( X_RFR_WIDTH ),
        .X_RFW_WIDTH ( X_RFW_WIDTH ),
        .X_MISA      ( X_MISA      )
    ) vcore_xif ();
    logic        vect_pending_load;
    logic        vect_pending_store;

    //signals for calculating the avg VL.  Defined here in case vcore is not instantiated
    logic        vcore_result_valid /* verilator public */;
    logic        vcore_result_ready /* verilator public */;
    assign vcore_result_valid = vcore_xif.result_valid;
    assign vcore_result_ready = vcore_xif.result_ready;
    logic [31:0] csr_vtype_o /* verilator public */;
    logic [31:0] csr_vl_o /* verilator public */;
    logic [31:0] csr_vlen_b_o /* verilator public */;

    // CSR register interface for Vector Unit
    localparam int unsigned VECT_CSR_CNT = 7;
    logic [11:0] vect_csr_addr [VECT_CSR_CNT];
    logic [31:0] vect_csr_rdata[VECT_CSR_CNT];
    logic        vect_csr_we   [VECT_CSR_CNT];
    logic [31:0] vect_csr_wdata[VECT_CSR_CNT];
    assign vect_csr_addr = '{
        12'h008, // vstart
        12'h009, // vxsat
        12'h00A, // vxrm
        12'h00F, // vcsr
        12'hC20, // vl
        12'hC21, // vtype
        12'hC22  // vlenb
    };


    //localparam bit USE_XIF_MEM = VMEM_W == 32;
    localparam bit USE_XIF_MEM = '0; // Force Vicuna to always use direct memory port and not XIF interface (brings system closer to updated XIF compliance)

    logic [3:0] commit_id_test;

    assign commit_id_test = host_xif.commit.id;

     logic [3:0] result_id_test;

    assign result_id_test = host_xif.result.id;

    
    `ifdef XIF_ON
    localparam bit X_EXT = 1'b1;
    `else
    localparam bit X_EXT = 1'b0;
    `endif

    // eXtension Interface
    if_xif #(
        .X_NUM_RS    ( 3  ),
        .X_MEM_WIDTH ( 32 ),
        .X_RFR_WIDTH ( 32 ),
        .X_RFW_WIDTH ( 32 ),
        .X_MISA      ( 32'b00000000000000000000000000100000 )
    ) host_xif();

    cv32e40x_core #(
        .X_EXT               ( X_EXT         ),
        .X_NUM_RS            ( 3             )
    ) core (
        .clk_i               ( clk_i         ),
        .rst_ni              ( rst_ni        ),
        .scan_cg_en_i        ( 1'b0          ),
        .boot_addr_i         ( 32'h00000080  ),
        .dm_exception_addr_i ( '0            ),
        .dm_halt_addr_i      ( '0            ),
        .mhartid_i           ( '0            ),
        .mimpid_patch_i      ( '0            ),
        .mtvec_addr_i        ( 32'h00000000  ),
        .instr_req_o         ( instr_req     ),
        .instr_gnt_i         ( instr_gnt     ),
        .instr_rvalid_i      ( instr_rvalid  ),
        .instr_addr_o        ( instr_addr    ),
        .instr_memtype_o     (               ),
        .instr_prot_o        (               ),
        .instr_dbg_o         (               ),
        .instr_rdata_i       ( instr_rdata   ),
        .instr_err_i         ( instr_err     ),
        .data_req_o          ( sdata_req     ),
        .data_gnt_i          ( sdata_gnt     ),
        .data_rvalid_i       ( sdata_rvalid  ),
        .data_addr_o         ( sdata_addr    ),
        .data_be_o           ( sdata_be      ),
        .data_we_o           ( sdata_we      ),
        .data_wdata_o        ( sdata_wdata   ),
        .data_memtype_o      (               ),
        .data_prot_o         (               ),
        .data_dbg_o          (               ),
        .data_atop_o         (               ),
        .data_rdata_i        ( sdata_rdata   ),
        .data_err_i          ( sdata_err     ),
        .data_exokay_i       ( 1'b0          ),
        .mcycle_o            (               ),
        .xif_compressed_if   ( host_xif      ),
        .xif_issue_if        ( host_xif      ),
        .xif_commit_if       ( host_xif      ),
        .xif_mem_if          ( host_xif      ),
        .xif_mem_result_if   ( host_xif      ),
        .xif_result_if       ( host_xif      ),
        //.xif_result_id       ( host_xif.result.id     ),
        .irq_i               ( '0            ),
        .clic_irq_i          ( '0            ),
        .clic_irq_id_i       ( '0            ),
        .clic_irq_level_i    ( '0            ),
        .clic_irq_priv_i     ( '0            ),
        .clic_irq_shv_i      ( '0            ),
        .fencei_flush_req_o  (               ),
        .fencei_flush_ack_i  ( 1'b0          ),
        .debug_req_i         ( 1'b0          ),
        .debug_havereset_o   (               ),
        .debug_running_o     (               ),
        .debug_halted_o      (               ),
        .fetch_enable_i      ( 1'b1          ),
        .core_sleep_o        (               )
    );

    //CONNECTING VPROC_XIF to HOST_XIF.
    assign vcore_xif.issue_valid         = host_xif.issue_valid;           
    assign host_xif.issue_ready          = vcore_xif.issue_ready;          
    assign vcore_xif.issue_req.instr     = host_xif.issue_req.instr;        
    assign vcore_xif.issue_req.mode      = host_xif.issue_req.mode;         
    assign vcore_xif.issue_req.id        = host_xif.issue_req.id;           
    assign vcore_xif.issue_req.rs        = host_xif.issue_req.rs;           
    assign vcore_xif.issue_req.rs_valid  = host_xif.issue_req.rs_valid;     
    assign host_xif.issue_resp.accept    = vcore_xif.issue_resp.accept;     
    assign host_xif.issue_resp.writeback = vcore_xif.issue_resp.writeback;  
    assign host_xif.issue_resp.dualwrite = vcore_xif.issue_resp.dualwrite;  
    assign host_xif.issue_resp.dualread  = vcore_xif.issue_resp.dualread;   
    assign host_xif.issue_resp.loadstore = vcore_xif.issue_resp.loadstore;  
    assign host_xif.issue_resp.exc       = vcore_xif.issue_resp.exc;        

    assign vcore_xif.commit_valid       = host_xif.commit_valid;            
    assign vcore_xif.commit.id          = host_xif.commit.id;               
    assign vcore_xif.commit.commit_kill = host_xif.commit.commit_kill;      

    assign host_xif.result_valid   = vcore_xif.result_valid;
    assign vcore_xif.result_ready  = host_xif.result_ready;                 
    assign host_xif.result.id      = vcore_xif.result.id;                   
    assign host_xif.result.data    = vcore_xif.result.data;                    
    assign host_xif.result.rd      = vcore_xif.result.rd;                   
    assign host_xif.result.we      = vcore_xif.result.we;                   
    assign host_xif.result.exc     = vcore_xif.result.exc;                  
    assign host_xif.result.exccode = vcore_xif.result.exccode;              
    assign host_xif.result.err     = vcore_xif.result.err;                  
    assign host_xif.result.dbg     = vcore_xif.result.dbg;                  

    if (USE_XIF_MEM) begin
        assign host_xif.mem_valid         = vcore_xif.mem_valid;
        assign vcore_xif.mem_ready        = host_xif.mem_ready;             
        assign host_xif.mem_req.id        = vcore_xif.mem_req.id;           
        assign host_xif.mem_req.addr      = vcore_xif.mem_req.addr;         
        assign host_xif.mem_req.mode      = vcore_xif.mem_req.mode;         
        assign host_xif.mem_req.we        = vcore_xif.mem_req.we;           
        assign host_xif.mem_req.size      = vcore_xif.mem_req.size;         
        assign host_xif.mem_req.be        = vcore_xif.mem_req.be;           
        assign host_xif.mem_req.attr      = vcore_xif.mem_req.attr;         
        assign host_xif.mem_req.wdata     = vcore_xif.mem_req.wdata;        
        assign host_xif.mem_req.last      = vcore_xif.mem_req.last;         
        assign host_xif.mem_req.spec      = vcore_xif.mem_req.spec;         
        assign vcore_xif.mem_resp.exc     = host_xif.mem_resp.exc;          
        assign vcore_xif.mem_resp.exccode = host_xif.mem_resp.exccode;      
        assign vcore_xif.mem_resp.dbg     = host_xif.mem_resp.dbg;          
        assign vcore_xif.mem_result_valid = host_xif.mem_result_valid;      
        assign vcore_xif.mem_result.id    = host_xif.mem_result.id;         
        assign vcore_xif.mem_result.rdata = host_xif.mem_result.rdata;      
        assign vcore_xif.mem_result.err   = host_xif.mem_result.err;        
        assign vcore_xif.mem_result.dbg   = host_xif.mem_result.dbg;        
    end

    assign vect_csr_we    = '{default:'0};
    assign vect_csr_wdata = '{default:'0};


    ///////////////////////////////////////////////////////////////////////////
    // VECTOR CORE INTEGRATION

    // Vector CSR read/write conversion
    logic [31:0] csr_vtype;
    assign csr_vtype_o = csr_vtype;
    logic [31:0] csr_vl;
    assign csr_vl_o = csr_vl;
    logic [31:0] csr_vlenb;
    assign csr_vlen_b_o = csr_vlenb;
    logic [31:0] csr_vstart_rd;
    logic [31:0] csr_vstart_wr;
    logic        csr_vstart_wren;
    logic        csr_vxsat_rd;
    logic        csr_vxsat_wr;
    logic        csr_vxsat_wren;
    logic [1:0]  csr_vxrm_rd;
    logic [1:0]  csr_vxrm_wr;
    logic        csr_vxrm_wren;
    assign vect_csr_rdata[0] = csr_vstart_rd;
    assign vect_csr_rdata[1] = {31'b0, csr_vxsat_rd};
    assign vect_csr_rdata[2] = {30'b0, csr_vxrm_rd};
    assign vect_csr_rdata[3] = {29'b0, csr_vxrm_rd, csr_vxsat_rd};
    assign vect_csr_rdata[4] = csr_vl;
    assign vect_csr_rdata[5] = csr_vtype;
    assign vect_csr_rdata[6] = csr_vlenb;
    assign csr_vstart_wr     = vect_csr_wdata[0];
    assign csr_vstart_wren   = vect_csr_we[0];
    assign csr_vxsat_wr      = vect_csr_we[1] ? vect_csr_wdata[1][0]   : vect_csr_wdata[3][0];
    assign csr_vxsat_wren    = vect_csr_we[1] | vect_csr_we[3];
    assign csr_vxrm_wr       = vect_csr_we[2] ? vect_csr_wdata[2][1:0] : vect_csr_wdata[3][2:1];
    assign csr_vxrm_wren     = vect_csr_we[2] | vect_csr_we[3];


    // Data read/write for Vector Unit
    logic                vdata_gnt;
    logic                vdata_rvalid;
    logic                vdata_err;
    logic [VMEM_W-1:0]   vdata_rdata;
    logic                vdata_req;
    logic [31:0]         vdata_addr;
    logic                vdata_we;
    logic [VMEM_W/8-1:0] vdata_be;
    logic [VMEM_W-1:0]   vdata_wdata;
    logic [X_ID_WIDTH-1:0] vdata_req_id;
    logic [X_ID_WIDTH-1:0] vdata_res_id;

    // Allow for vector loads/stores to be misaligned with respect to VMEM_W
    `ifdef FORCE_ALIGNED_READS
    localparam bit [VLSU_FLAGS_W-1:0] VLSU_FLAGS = (VLSU_FLAGS_W'(1) << VLSU_ALIGNED_UNITSTRIDE);
    `else
    localparam bit [VLSU_FLAGS_W-1:0] VLSU_FLAGS = (VLSU_FLAGS_W'(0) << VLSU_ALIGNED_UNITSTRIDE);
    `endif

    localparam bit [BUF_FLAGS_W -1:0] BUF_FLAGS  = (BUF_FLAGS_W'(1) << BUF_DEQUEUE  ) |
                                                   (BUF_FLAGS_W'(1) << BUF_VREG_PEND);
`ifdef RISCV_ZVE32X
    vproc_core #(
        .XIF_ID_W           ( X_ID_WIDTH         ),
        .XIF_MEM_W          ( VMEM_W             ),
        .VREG_TYPE          ( VREG_TYPE          ),
        .MUL_TYPE           ( MUL_TYPE           ),
        .VLSU_FLAGS         ( VLSU_FLAGS         ),
        .BUF_FLAGS          ( BUF_FLAGS          ),
        .DONT_CARE_ZERO     ( 1'b0               ),
        .ASYNC_RESET        ( 1'b0               )
    ) v_core (
        .clk_i              ( clk_i              ),
        .rst_ni             ( sync_rst_n         ),

        .xif_issue_if       ( vcore_xif          ),
        .xif_commit_if      ( vcore_xif          ),
        .xif_mem_if         ( vcore_xif          ),
        .xif_memres_if      ( vcore_xif          ),
        .xif_result_if      ( vcore_xif          ),

        .pending_load_o     ( vect_pending_load  ),
        .pending_store_o    ( vect_pending_store ),

        .csr_vtype_o        ( csr_vtype          ),
        .csr_vl_o           ( csr_vl             ),
        .csr_vlenb_o        ( csr_vlenb          ),
        .csr_vstart_o       ( csr_vstart_rd      ),
        //.csr_vstart_i       ( csr_vstart_wr      ), //unused
        //.csr_vstart_set_i   ( csr_vstart_wren    ),
        .csr_vxrm_o         ( csr_vxrm_rd        ),
        //.csr_vxrm_i         ( csr_vxrm_wr        ),
        //.csr_vxrm_set_i     ( csr_vxrm_wren      ),
        .csr_vxsat_o        ( csr_vxsat_rd       ),
        //.csr_vxsat_i        ( csr_vxsat_wr       ),
        //.csr_vxsat_set_i    ( csr_vxsat_wren     ),

        .pend_vreg_wr_map_o ( pend_vreg_wr_map_o )
    );



`endif

    // Extract vector unit memory signals from extension interface
    if (USE_XIF_MEM) begin
        assign vdata_req                  = '0;
        assign vdata_addr                 = '0;
        assign vdata_we                   = '0;
        assign vdata_be                   = '0;
        assign vdata_wdata                = '0;
        assign vdata_req_id               = '0;
    end else begin
        assign vdata_req                  = vcore_xif.mem_valid;
        assign vcore_xif.mem_ready        = vdata_gnt;
        assign vdata_addr                 = vcore_xif.mem_req.addr;
        assign vdata_we                   = vcore_xif.mem_req.we;
        assign vdata_be                   = vcore_xif.mem_req.be;
        assign vdata_wdata                = vcore_xif.mem_req.wdata;
        assign vdata_req_id               = vcore_xif.mem_req.id;
        assign vcore_xif.mem_resp.exc     = '0;
        assign vcore_xif.mem_resp.exccode = '0;
        assign vcore_xif.mem_resp.dbg     = '0;
        assign vcore_xif.mem_result_valid = vdata_rvalid;
        assign vcore_xif.mem_result.id    = vdata_res_id;
        assign vcore_xif.mem_result.rdata = vdata_rdata;
        assign vcore_xif.mem_result.err   = vdata_err;
        assign vcore_xif.mem_result.dbg   = '0;
    end

    // Data arbiter for main core and vector unit
    logic                sdata_hold;
    logic                data_req;
    logic [31:0]         data_addr;
    logic                data_we;
    logic [VMEM_W/8-1:0] data_be;
    logic [VMEM_W  -1:0] data_wdata;
    logic                data_gnt;
    logic                data_rvalid;
    logic                data_err;
    logic [VMEM_W  -1:0] data_rdata;
    logic                sdata_waiting, vdata_waiting;
    logic [31:0]         sdata_wait_addr;
    logic [X_ID_WIDTH-1:0] vdata_wait_id;
    assign sdata_hold = ~USE_XIF_MEM & (vdata_req | vect_pending_store | (vect_pending_load & sdata_we));
    always_comb begin
        data_req   = vdata_req | (sdata_req & ~sdata_hold);
        data_addr  = sdata_addr;
        data_we    = sdata_we;
        
        `ifdef FORCE_ALIGNED_READS
        data_be    = {{(VMEM_W-32){1'b0}}, sdata_be} << (sdata_addr[$clog2(VMEM_W/8)-1:0] & {{$clog2(VMEM_W/32){1'b1}}, 2'b00});
        data_wdata = '0;
        for (int i = 0; i < VMEM_W / 32; i++) begin
            data_wdata[32*i +: 32] = sdata_wdata;
        end
        `else
        data_be    = {{(VMEM_W-32){1'b0}}, sdata_be};
        data_wdata = {{(VMEM_W-32){1'b0}}, sdata_wdata};
        `endif
        
        if (vdata_req) begin
            data_addr  = vdata_addr;
            data_we    = vdata_we;
            data_be    = vdata_be;
            data_wdata = vdata_wdata;
        end
    end
    assign sdata_gnt = data_gnt & sdata_req & ~sdata_hold;
    assign vdata_gnt = data_gnt & vdata_req;
    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (~rst_ni) begin
            sdata_waiting   <= 1'b0;
            vdata_waiting   <= 1'b0;
            sdata_wait_addr <= '0;
            vdata_wait_id   <= '0;
        end else begin
            if (sdata_gnt) begin
                sdata_waiting   <= 1'b1;
                sdata_wait_addr <= sdata_addr;
            end
            else if (sdata_rvalid) begin
                sdata_waiting <= 1'b0;
            end
            if (vdata_gnt) begin
                vdata_waiting <= 1'b1;
                vdata_wait_id <= vdata_req_id;
            end
            else if (vdata_rvalid) begin
                vdata_waiting <= 1'b0;
            end
        end
    end
    assign sdata_rvalid = sdata_waiting & data_rvalid;
    assign vdata_rvalid = vdata_waiting & data_rvalid;
    assign sdata_err    = data_err;
    assign vdata_err    = data_err;

    `ifdef FORCE_ALIGNED_READS
    assign sdata_rdata  = data_rdata[(sdata_wait_addr[$clog2(VMEM_W)-1:0] & {3'b000, {($clog2(VMEM_W/8)-2){1'b1}}, 2'b00})*8 +: 32];
    `else
    assign sdata_rdata  = data_rdata[31:0];
    `endif
    assign vdata_rdata  = data_rdata;
    assign vdata_res_id = vdata_wait_id;

    // Memory Interface signals I-DATA
    logic             imem_req;
    logic             imem_gnt;
    logic [31:0]      imem_addr;
    logic             imem_rvalid;
    logic [MEM_W-1:0] imem_rdata;
    logic             imem_err;
    logic             i_miss /* verilator public */;
    logic             i_hit  /* verilator public */;
    
    assign imem_req     = instr_req;
    assign imem_addr    = instr_addr;
    assign instr_gnt    = imem_gnt;
    assign instr_rvalid = imem_rvalid;
    assign instr_rdata  = imem_rdata[31:0];
    assign instr_err    = imem_err;


    // Memory Interface signals D-DATA
    logic               dmem_req;
    logic               dmem_gnt;
    logic [31:0]        dmem_addr;
    logic               dmem_we;
    logic [MEM_W/8-1:0] dmem_be;
    logic [MEM_W  -1:0] dmem_wdata;
    logic               dmem_rvalid;
    logic               dmem_wvalid;
    logic [MEM_W  -1:0] dmem_rdata;
    logic               dmem_err;
    logic               d_miss /* verilator public */;
    logic               d_hit  /* verilator public */;

    assign dmem_req    = data_req;
    assign dmem_addr   = data_addr;
    assign dmem_we     = data_we;
    assign dmem_be     = data_be;
    assign dmem_wdata  = data_wdata;
    assign data_gnt    = dmem_gnt;
    assign data_rvalid = dmem_rvalid | dmem_wvalid;
    assign data_rdata  = dmem_rdata;
    assign data_err    = dmem_err;




    ///////////////////////////////////////////////////////////////////////////
    // MEMORY ARBITER // Is tracking sources necessary now that caches removed?
    

    //if cache is not enabled, no memory arbitration required
    always_comb begin

        mem_req_o   = dmem_req;
        mem_be_o    = dmem_be;
        mem_wdata_o = dmem_wdata;
        mem_we_o    = dmem_we;
        mem_addr_o  = dmem_addr;
            
    end

    assign dmem_gnt =  dmem_req;


    always_comb begin
        mem_ireq_o   = imem_req;
        mem_iaddr_o = imem_addr;       
    end

    assign imem_gnt =  imem_req;


    // shift register keeping track of the source of mem requests for up to 32 cycles (needed to keep track of reads/writes)
    logic        req_sources  [32];
    logic        req_write    [32]; // keeping track of whether the request was a write
    logic [4:0]  req_count;
    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (~rst_ni) begin
            req_count <= '0;
        end else begin
            if (mem_rvalid_i) begin
                for (int i = 0; i < 31; i++) begin
                    req_sources  [i] <= req_sources  [i+1];
                    req_write    [i] <= req_write    [i+1];
                end
                if (~dmem_gnt) begin
                    req_count <= req_count - 1;
                end else begin
                    req_sources  [req_count-1] <= dmem_gnt;
                    req_write    [req_count-1] <= dmem_we;
                end
            end
            else if (dmem_gnt) begin
                req_sources  [req_count] <= dmem_gnt;
                req_write    [req_count] <= dmem_we;
                req_count                <= req_count + 1;
            end
        end
    end

    assign imem_rvalid = mem_irvalid_i;

    assign dmem_rvalid = mem_rvalid_i & ~req_write[0];
    assign dmem_wvalid = mem_rvalid_i &  req_write[0]; //this could be an issue?

    assign imem_err    = mem_ierr_i;
    assign dmem_err    = mem_err_i;

    assign imem_rdata  = mem_irdata_i;
    assign dmem_rdata  = mem_rdata_i;




endmodule
