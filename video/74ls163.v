module 74ls163(i, ld, clr, clk, p, t, rco, q);

   input [3:0] i;
   input       ld, clr, p, t, clk;
   output      rco, [3:0]q;

   reg 	       count;

   count = 0;


   always@(p, t, ld, clr, posedge clk)
     if(~p & ~t)
       if(~ld)
	 q[3:0] = i[3:0];
   
       else if(~clr)
	 q[3:0] = 4b'0000;
   
   rco = 0;


   begin
      while (count <= 15)
	q+= 1'b1;
      
      count++;
      
      if(count == 15)
	count = 0;
      
      q == 4'b0000;
      
      rco = 1'b1;
      
   end

endmodule // ls163
