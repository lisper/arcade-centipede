//
//
//

`timescale 1ns/1ns

`define use_vga
`define waves

module centipede_top_tb;

   parameter CLK_frequency  = 12.0e6;
   parameter CLK_period_f   = 1.0 /*sec*/ / CLK_frequency;
   parameter CLK_period     = CLK_period_f * 1000000000;

   reg clk;
   reg reset;

   wire [8:0] rgb;
   wire       csync, hsync, vsync, hblank, vblank;
   wire [7:0] audio;
   wire [3:0] led;

   reg [7:0]  trakball;
   reg [7:0]  joystick;
   reg [7:0]  sw1;
   reg [7:0]  sw2;
   reg [9:0] playerinput;

   wire      clk6m;
   
   centipede uut(
		 .clk_12mhz(clk),
 		 .reset(reset),
		 .playerinput_i(playerinput),
		 .trakball_i(trakball),
		 .joystick_i(joystick),
		 .sw1_i(sw1),
		 .sw2_i(sw2),
		 .led_o(led),
		 .rgb_o(rgb),
		 .sync_o(csync),
		 .hsync_o(hsync),
		 .vsync_o(vsync),
		 .hblank_o(hblank),
		 .vblank_o(vblank),
		 .audio_o(audio),
		 .clk_6mhz_o(clk6m)
		 );
   
   always
     begin
        clk = 1'b0;
        //#(CLK_period/2);
        #5;
        clk = 1'b1;
        //#(CLK_period/2);
        #5;
     end

   //
`ifdef waves
   initial
     begin
	$timeformat(-9, 0, "", 7);
	$dumpfile("centipede_tb.vcd");
	$dumpvars(0, centipede_top_tb);
     end
`endif

   initial
     begin
	clk = 0;
	reset = 0;

	trakball = 0;
	joystick = 0;

	sw1 = 8'h54;
	sw2 = 8'b0;

	playerinput = 10'b000_000_00_00;
	playerinput = 10'b000_100_00_00;
	playerinput = 10'b111_101_11_11;
	
	#100 reset = 1;
	#20 reset = 0;

//	#10000;
//	#100000;
//	#10000000;
	
//       #100000000;
//       #25000000;
//       $finish;
     end

   
`ifdef use_vga
   initial
     begin
	$cv_init_vga(800, 600);
     end

   wire [7:0] rgb8;
//`define map
 `ifdef map
   assign rbg8 = rgb != 0 ? 8'hff : 8'h0;
 `else
   assign rgb8 = (hblank | vblank) ? 8'b0 : { rgb[8:6],  rgb[4:3],  rgb[2:0] };
//      assign rgb8 = (hblank | vblank) ? 8'b0 : { rgb[2:0],  rgb[4:3],  rgb[8:6] };
 `endif 
   
   always @(posedge clk)
     begin
	$cv_clk_vga(vsync, hsync, rgb8);
     end
`endif
   
endmodule // ff_tb


