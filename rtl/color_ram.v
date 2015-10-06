
module color_ram (input [3:0] a,
		  output [3:0] dout,
		  input [3:0]  din,
		  input        w_n);


   reg [3:0] ram[0:15];
   reg [3:0] d;

   integer    j;
   
   initial
     begin
	for (j = 0; j < 16; j = j + 1)
	  ram[j] = 0;
     end

   assign dout = d;
   
   always @(a)
     d = ram[a];

   always @(a or w_n)
     if (~w_n)
       ram[a] = din;

endmodule

