# arcade-centipede

verilog fpga centipede arcade games - from the schematics
some liberties taken to convert to sync rams

## Overview

Ports of original arcade hardware/software to the "Pipistrello" FPGA board @ http://pipistrello.saanlima.com

The board features a Xilinx Spartan-6 LX45 with HDMI output.  I made a
"control panel" with two joysticks and buttons,

http://www.robotron2084guidebook.com/technical/gameplatforms/mame/homemadecontrolpaneltips/

And then wired it to the FPGA;  the inputs are pulled up and the switch contacts connect to ground.

I verilog creates a "mapping" of the video from the old CGA-ish format
to 640x480 VGA.  Video output is to the HDMI port and audio (sound) to
the sound port.  Nothing like pokey noises to make your day.

There are several similators and basic cpu co-simulation to make sure
the rtl cpu was doing the right thing.  There is also verilog
simulator PLI code to output the video in simulation.

"cver" and "verilator" were used to simulate the RTL.

