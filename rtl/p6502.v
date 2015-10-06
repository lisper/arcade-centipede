
//`define no_cpu
`define bc_cpu

//`define orig_cpu
//`define otten_cpu
//`define ac_cpu

`ifdef orig_cpu
 `define HALFCYCLE 25
 `define W 6
 `include "../6502/orig/models.v"
 `include "../6502/orig/chip_6502.v"
 `include "../6502/orig/clocks_6502.v"
`endif

`ifdef bc_cpu
 `include "../6502/bc/bc6502.v"
 `include "../6502/bc/addsub.v"
`endif

`ifdef otten_cpu
 `include "../6502/otten/cpu.v"
 `include "../6502/otten/ALU.v"
`endif

`ifdef ac_cpu
 `include "../6502/ac6502-0.6/ac6502_cfg.v"
 `include "../6502/ac6502-0.6/adder4.v"
 `include "../6502/ac6502-0.6/add_16_8.v"
 `include "../6502/ac6502-0.6/ac6502_addsub.v"
 `include "../6502/ac6502-0.6/ac6502_defs.v"
 `include "../6502/ac6502-0.6/ac6502_fetch.v"
 `include "../6502/ac6502-0.6/ac6502_bus2memr.v"
 `include "../6502/ac6502-0.6/ac6502_exe_alu.v"
 `include "../6502/ac6502-0.6/ac6502_bus2mem.v"
 `include "../6502/ac6502-0.6/ac6502_exe_ctrl.v"
 `include "../6502/ac6502-0.6/ac6502_bus_mux.v"
 `include "../6502/ac6502-0.6/ac6502_exe_data.v"
 `include "../6502/ac6502-0.6/ac6502_exe.v"
 `include "../6502/ac6502-0.6/ac6502_top.v"
`endif

module p6502(
	     input 	   clk, 
	     input 	   reset_n,
	     input 	   nmi,
	     input 	   irq,
	     input 	   so,
	     input 	   rdy,
	     input 	   phi0,
	     output 	   phi2,
	     output 	   rw_n,
	     output [15:0] a,
	     input [7:0]   din,
	     output [7:0]  dout
	     );

`ifdef no_cpu
   assign rw_n = 1'b1;
   assign a = 0;
   assign dout = 0;
`endif

`ifdef orig_cpu
   wire [15:0] ab;
   wire        res, clk0, reset;
   wire        clk1out, clk2out;
   wire        rw;
   wire [7:0]  db_i;
   wire [7:0]  db_o;
   wire [7:0]  db_t;
   
   clocks_6502 _clocks_6502(eclk, ereset, res, clk0);

   chip_6502 _chip_6502(eclk, ereset,
     ab[0], ab[1], ab[2], ab[3], ab[4], ab[5], ab[6], ab[7], ab[8], ab[9], ab[10], ab[11], ab[12], ab[13], ab[14], ab[15],
     db_i[0], db_o[0], db_t[0], db_i[1], db_o[1], db_t[1], db_i[2], db_o[2], db_t[2], db_i[3], db_o[3], db_t[3], 
     db_i[4], db_o[4], db_t[4], db_i[5], db_o[5], db_t[5], db_i[6], db_o[6], db_t[6], db_i[7], db_o[7], db_t[7], 
     res, rw, sync, so, clk0, clk1out, clk2out, rdy, ~nmi, ~irq);

   assign eclk = clk;
   assign ereset = ~reset_n;
//   assign clk0 = phi0;
   assign db_i = din;
   
   assign a = ab;
   assign rw_n = rw;
   assign phi2 = clk2out;
   assign dout = db_o;

   always @(posedge eclk)
     $display("%t; reset_n=%b ab=%x din=%x rw_n=%b", $time, reset_n, ab, din, rw_n);
   
`endif

`ifdef bc_cpu
   wire [15:0] ma;
   wire        reset;
   wire        rw;

   wire        rw_nxt;
   wire [15:0] ma_nxt;
   wire        sync;
   wire [31:0] state;
   wire [4:0]  flags;
   
   bc6502 bc6502(reset, phi0, ~nmi, ~irq, rdy, so, din, dout, rw, ma,
		 rw_nxt, ma_nxt, sync, state, flags);

   assign reset = ~reset_n;
   assign a = ma;
   assign rw_n = rw;
//   assign phi2 = clk;
   assign phi2 = ~phi0;

   //
   integer     pccount;
   initial
     pccount = 0;

   always @(posedge clk)
     begin
	if (bc6502.s_sync)
	  begin
	     pccount = pccount + 1;
	     if (pccount == 1000/* || $time > 9999999*/)
	       begin
		  pccount = 0;
		  $display("%t; cpu: pc %x; a=%x x=%x", $time, bc6502.pc_reg, bc6502.a_reg, bc6502.x_reg);
`ifndef verilator
		  $fflush;
		  $flushlog;
`endif
	       end

	     if (^bc6502.pc_reg === 1'bX || ^bc6502.a_reg === 1'bX || ^bc6502.x_reg === 1'bX)
	       begin
		  $display("%t; x's in pc", $time);
		  $finish;
	       end

	     if (^a === 1'bX || ^din === 1'bX || ^dout === 1'bX)
	       begin
		  $display("%t; x's in addr bus or data bus", $time);
		  $finish;
	       end
	  end
     end
   
`endif

`ifdef otten_cpu
   wire [15:0] ab;
   wire        reset;
   wire        we;

   wire        sync;
   wire [31:0] state;
   wire [4:0]  flags;
   
   cpu cpu6502(
	       .clk(clk),
	       .reset(reset),
	       .AB(a),
	       .DI(din),
	       .DO(dout),
	       .WE(we),
	       .IRQ(irq),
	       .NMI(~nmi),
	       .RDY(rdy)
	       );

   assign reset = ~reset_n;
   assign rw_n = ~we;
   assign phi2 = clk;

   //
   integer     pccount;
   initial
     pccount = 0;

   assign sync = (cpu6502.state == 6'd12) & (cpu6502.RDY);
   
   always @(posedge clk)
     begin
	if (sync)
	  begin
	     pccount = pccount + 1;
	     if (pccount == 1000/* || $time > 9999999*/ || 1)
	       begin
		  pccount = 0;
		  $display("%t; cpu: pc %x; a=%x x=%x", $time, cpu6502.PC, cpu6502.AXYS[0], cpu6502.AXYS[1]);
`ifndef verilator
		  $fflush;
		  $flushlog;
`endif
	       end

	     if (^cpu6502.PC === 1'bX || ^cpu6502.AXYS[0] === 1'bX || ^cpu6502.AXYS[1] === 1'bX)
	       begin
		  $display("%t; x's in pc", $time);
		  $finish;
	       end

	     if (^a === 1'bX || ^din === 1'bX || ^dout === 1'bX)
	       begin
		  $display("%t; x's in addr bus or data bus", $time);
		  $finish;
	       end
	  end
     end
`endif //  `ifdef otten_cpu

`ifdef ac_cpu
   wire        we;
   wire        en, ack;
   
   ac6502_top cpu(
		  .clk(clk),
		  .rst(reset_n),
		  .irq(irq),
		  .nmi(nmi),
		  .addr(a),
		  .wdata(dout),
		  .rdata(din),
		  .en(en),
		  .wen(we),
		  .ack(ack)
		  );

//   assign ack = 1'b0/*reset_n*//*1'b1*/;
   assign rw_n = ~we;
   assign phi2 = clk;

   reg [2:0] w;
   always @(posedge clk)
     if (~reset_n)
       w <= 0;
     else
       w <= { w[1:0], en};
   assign ack = w[2];
   
   always @(posedge clk)
     $display("%t; reset_n=%b irq=%b nmi=%b a=%x we=%b di=%x do=%x en %b ack %b",
	      $time, reset_n, irq, nmi, a, we, din, dout, en, ack);
   
`endif
   
endmodule // p6502
