//
//
//

`timescale 1ns/1ns

`define use_vga
//`define waves

module centipede_top_tb;

   reg clk_50m, clk_25m, clk_12m, clk_6m;
   wire [5:1] led;
   wire       vga_hsync, vga_vsync, vga_r, vga_g, vga_b;
   reg       switch, button1, button2, button3;
   
   cent_top top(
		.led(led),
		.sysclk(clk_50m),
		.clk_vga(clk_25m),
		.clk_cpu(clk_12m),
		.clk_pix(clk_6m),

		.vga_hsync(vga_hsync),
		.vga_vsync(vga_vsync),
		.vga_r(vga_r),
		.vga_g(vga_g),
		.vga_b(vga_b),
		 
		.switch(switch),
		.button1(button1),
		.button2(button2),
		.button3(button3)
		);

   always
     begin
        clk_50m = 1'b0;
        #10;
        clk_50m = 1'b1;
        #10;
     end

   initial
     clk_25m = 0;
   
   always @(posedge clk_50m)
     clk_25m <= ~clk_25m;

   always
     begin
        clk_12m = 1'b0;
        #41;
        clk_12m = 1'b1;
        #42;
     end
   
   initial
     clk_6m = 0;
   
   always @(posedge clk_12m)
     clk_6m <= ~clk_6m;

   //
`ifdef waves
   initial
     begin
	$timeformat(-9, 0, "", 7);
	$dumpfile("centipede_tb.vcd");
	$dumpvars(0, centipede_top_tb);
     end
`endif

`ifdef never
   always
     begin
	$display("%t; clk_50m %b clk_25m %b clk_12m %b clk_6m %b",
		 $time, clk_50m, clk_25m, clk_12m, clk_6m);
	#10;
     end
`endif
   
   initial
     begin
	clk_50m = 0;
	clk_25m = 0;
	clk_12m = 0;
	clk_6m = 0;
	
	switch = 0;
	button1 = 0;
	button2 = 0;
	button3 = 0;
     end

   
`ifdef use_vga
   initial
     begin
	$cv_init_vga(800, 600);
     end

   wire [7:0] rgb8;
   wire [2:0] red, green, blue;

   assign red   = vga_r ? 3'b111 : 0;
   assign green = vga_g ? 3'b111 : 0;
   assign blue  = vga_b ? 3'b111 : 0;
   
   assign rgb8 = { red, green[1:0], blue };
   
   always @(posedge clk_25m)
     begin
	$cv_clk_vga(vga_vsync, vga_hsync, rgb8);
     end
`endif
   
endmodule // ff_tb


