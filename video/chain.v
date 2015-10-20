`timescale 1ns/1ns

`include "vprom.v"

module m74ls163a (clk, clr_n, ld, ep, et, d, q, rc);


   input clk, clr_n, ld, ep, et;
   input [3:0] d;
   output      rc;
   output [3:0] q;
   reg [3:0] 	q;
   reg 		rc;

   always @(posedge clk)
     if (clr_n == 0) q <= 4'b0000;
     else if (ld == 0) q <= d;
     else if (et & ep) q <= q + 1;

   always @(q or et)
     if (q == 15 && et == 1) rc = 1;
     else rc = 0;

endmodule


module m74ls163b(clk, clr_n, et, ep, ld, d, q, rc);

   parameter WID=4;

   input clk;
   input clr_n;   // clear active low
   input et;   // clock enable
   input ep;   // clock enable
   input ld;   // load active low
   input [WID:1] d;
   output [WID:1] q;
   reg [WID:1] 	  q;
   output 	  rc;

   assign rc = &{q[WID:1],et};

   always @(posedge clk)
     begin
	if (!clr_n)
	  q <= {WID{1'b0}};

	else if (!ld)
	  q <= d;

	else if (ep & et)
	  q <= q + {{WID-1{1'b0}},1'b1};

     end

endmodule

module tb;

   reg clk;
   reg reset_n;
   wire reset;

   wire [11:0] h;
   wire        hrc0, hrc1, hrc2;
   wire        hld2;
   
   wire        clk_12mhz;
   assign clk_12mhz = clk;
   
   m74ls163b hclk0(.clk(clk_12mhz), .clr_n(reset_n), .et(1'b1), .ep(1'b1), .ld(1'b1),
		   .d(4'b0000), .q(h[3:0]), .rc(hrc0));

   m74ls163b hclk1(.clk(clk_12mhz), .clr_n(reset_n), .et(hrc0), .ep(hrc0), .ld(1'b1),
		   .d(4'b0000), .q(h[7:4]), .rc(hrc1));

   m74ls163b hclk2(.clk(clk_12mhz), .clr_n(reset_n), .et(hrc1), .ep(hrc0), .ld(hld2),
		   .d(4'b1101), .q(h[11:8]), .rc(hrc2));

   assign hld2 = ~hrc2;
   

   wire       ss_6mhz, ss_1h, ss_2h, ss_4h, ss_8h, ss_16h, ss_32h, ss_64h, ss_128h, ss_256h;
   
   assign ss_6mhz  = h[0];
   assign ss_1h    = h[1];
   assign ss_2h    = h[2];
   assign ss_4h    = h[3];
   assign ss_8h    = h[4];
   assign ss_16h   = h[5];
   assign ss_32h   = h[6];
   assign ss_64h   = h[7];
   assign ss_128h  = h[8];
   assign ss_256h  = h[9];

   wire       ss_256h_n;
   assign ss_256h_n = ~ss_256h;
   
   //
   wire [7:0] v;
   wire        vrc0, vrc1;
   wire        vclk;
//   wire        ss_vreset_n;

   m74ls163b vclk0(.clk(vclk), .clr_n(reset_n), .et(1'b1), .ep(1'b1), .ld(1'b1/*ss_vreset_n*/),
		   .d(4'b0000), .q(v[3:0]), .rc(vrc0));

   m74ls163b vclk1(.clk(vclk), .clr_n(reset_n), .et(vrc0), .ep(vrc0), .ld(1'b1/*ss_vreset_n*/),
		   .d(4'b0000), .q(v[7:4]), .rc());

   assign vclk = ~ss_256h;
   
   wire        ss_1v, ss_2v, ss_4v, ss_8v, ss_16v, ss_32v, ss_64v, ss_128v;

   assign ss_1v = v[0];
   assign ss_2v = v[1];
   assign ss_4v = v[2];
   assign ss_8v = v[3];
   assign ss_16v = v[4];
   assign ss_32v = v[5];
   assign ss_64v = v[6];
   assign ss_128v = v[7];

   //
   wire [7:0] ss_vprom_addr;
   wire [3:0] ss_vprom_out;
   reg [3:0]  ss_vprom_reg;
   
   assign ss_vprom_addr = { ss_vblank, ss_128v, ss_64v, ss_32v, ss_8v, ss_4v, ss_2v, ss_1v };
   
   vprom ss_vrom(
		 .clk(clk_12mhz),
		 .a(ss_vprom_addr),
		 .d(ss_vprom_out)
		 );

   always @(posedge ss_256h_n or posedge reset)
     if (reset)
       ss_vprom_reg <= 0;
     else
       ss_vprom_reg <= ss_vprom_out;

   wire ss_vsync, ss_vsync_n;
   wire ss_vreset, ss_vreset_n;
   wire	ss_vblank, ss_vblank_n;

   assign ss_vsync = ss_vprom_reg[0];
   assign ss_vsync_n = ~ss_vprom_reg[0];

   assign ss_vreset = ss_vprom_reg[2];
   assign ss_vreset_n = ~ss_vprom_reg[2];

   assign ss_vblank = ss_vprom_reg[3];
   assign ss_vblank_n = ~ss_vprom_reg[3];
   
   //----------------------------------

   //
   reg [11:0]  h_counter;
   
   initial
     h_counter = 0;

   always @(posedge clk_12mhz)
       if (h_counter == 12'hfff)
	 h_counter <= 12'b110100000000;
       else
	 h_counter <= h_counter + 12'd1;

   //
   wire       s_6mhz, s_1h, s_2h, s_4h, s_8h, s_16h, s_32h, s_64h, s_128h, s_256h;
   
   assign s_6mhz  = h_counter[0];
   assign s_1h    = h_counter[1];
   assign s_2h    = h_counter[2];
   assign s_4h    = h_counter[3];
   assign s_8h    = h_counter[4];
   assign s_16h   = h_counter[5];
   assign s_32h   = h_counter[6];
   assign s_64h   = h_counter[7];
   assign s_128h  = h_counter[8];
   assign s_256h  = h_counter[9];

   // debug
   wire [8:0] offset_h;
   assign offset_h = { s_256h, s_128h, s_64h, s_32h, s_16h, s_8h, s_4h, s_2h, s_1h };

   reg hsync;
   reg hsync_clr_n;
   wire hsync_clr;

   //
   //  256h ~256h hsync_clr hsync_clr_n
   //    0    1       0         1
   //    1    0       1         0

   reg 	s_8h_d, s_32h_d, s_256h_n_d;
   wire s_8h_rise, s_32h_rise, s_256h_n_rise;
   
   always @(posedge s_6mhz)
     if (~reset_n)
       begin
	  s_8h_d <= 0;
	  s_32h_d <= 0;
	  s_256h_n_d <= 0;
       end
     else
       begin
	  s_8h_d <= s_8h;
	  s_32h_d <= s_32h;
	  s_256h_n_d <= s_256h_n;
       end
   
   assign s_8h_rise = ~s_8h_d & s_8h;
   assign s_32h_rise = ~s_32h_d & s_32h;
   assign s_256h_n_rise = ~s_256h_n_d & s_256h_n;
   
   always @(posedge s_6mhz/*clk_12mhz*/)
     if (~reset_n)
       hsync_clr_n <= 1'b1;
     else
       if (s_256h)
	 hsync_clr_n <= 1'b0;
       else
	 if (s_32h_rise)
	   hsync_clr_n <= ~s_64h;

   assign hsync_clr = ~hsync_clr_n;

   always @(posedge s_6mhz/*clk_12mhz*/)
     if (hsync_clr)
       hsync <= 1'b0;
     else
       if (s_8h_rise)
	 hsync <= s_32h;

   //----------------

   wire s_256h_n;
   assign s_256h_n = 1'b1 ^ s_256h;   

   //
   wire v_counter_reset;
   reg [7:0]  v_counter;

   assign v_counter_reset = reset | vreset_n == 0;
   
   always @(posedge s_256h_n or posedge reset)
     if (reset)
       v_counter <= 0;
     else
       /* ld# is on positive clock edge */
       if (vreset_n == 0)
	 v_counter = 0;
       else
	 v_counter <= v_counter + 8'd1;

   wire       s_1v, s_2v, s_4v, s_8v, s_16v, s_32v, s_64v, s_128v;

   assign s_1v    = v_counter[0];
   assign s_2v    = v_counter[1];
   assign s_4v    = v_counter[2];
   assign s_8v    = v_counter[3];
   assign s_16v   = v_counter[4];
   assign s_32v   = v_counter[5];
   assign s_64v   = v_counter[6];
   assign s_128v  = v_counter[7];

   wire vsync, vsync_n;
   wire vreset, vreset_n;
   wire	vblank, vblank_n;

   wire [7:0] vprom_addr;
   wire [3:0] vprom_out;
   reg [3:0]  vprom_reg;
   
   assign vprom_addr = { vblank, s_128v, s_64v, s_32v, s_8v, s_4v, s_2v, s_1v };
   
   vprom vrom(
		   .clk(clk_12mhz),
		   .a(vprom_addr),
		   .d(vprom_out)
		   );

   always @(posedge s_256h_n or posedge reset)
     if (reset)
       vprom_reg <= 0;
     else
       vprom_reg <= vprom_out;

   assign vsync = vprom_reg[0];
   assign vsync_n = ~vprom_reg[0];

   assign vreset = vprom_reg[2];
   assign vreset_n = ~vprom_reg[2];

   assign vblank = vprom_reg[3];
   assign vblank_n = ~vprom_reg[3];

   assign reset = ~reset_n;

   //
   initial
     begin
	$timeformat(-9, 0, "", 7);
	$dumpfile("chain.vcd");
	$dumpvars(0, tb);
     end

   always
     begin
        clk = 1'b0; #41;
        clk = 1'b1; #41;
     end

   initial
     begin
	reset_n = 0;
	#500;
	reset_n = 1;
	
	#64000000;
	$finish;
     end
   
endmodule // tb
