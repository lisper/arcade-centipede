
module rom(input [12:0] a,
	   output [7:0] dout,
	   input 	cs_n);

   reg [7:0] q;
   
always @(a)   
//`include "../roms/extract/rom_code_case.v"
`include "../roms/extract/rom_code_case_patched.v"

  assign dout = q;

`ifdef debug_rom
   always @(a)   
     if (cs_n == 0)
       $display("rom: rom[%x] -> %x", a, q);
`endif
   
endmodule // rom
