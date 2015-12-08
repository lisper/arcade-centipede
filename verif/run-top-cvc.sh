#!/bin/sh

#DP=../rtl/pf_ram_dp_async.v
DP=../rtl/pf_ram_dp.v

RTL="../rtl/centipede.v ../rtl/p6502.v ../rtl/pokey.v \
     ../rtl/ram.v ../rtl/rom.v \
     ../rtl/color_ram.v ../rtl/pf_rom.v $DP \
     ../rtl/vprom.v ../rtl/hs_ram.v \
     ../rtl/car_lx45.v ../rtl/scanconvert2_lx45.v ../rtl/ram_dp128kx8.v \
     ../rtl/cent_top.v"

INC="+incdir+../6502/orig +incdir+../rtl"
INC="+incdir+../6502/ac6502-0.6 +incdir+../rtl"

DEBUG="+define+debug=1 +define+SIMULATION=1"
#DEBUG=+define+SIMULATION=1

PLI=+loadvpi=../pli/vga/vga.so:vpi_compat_bootstrap

cvc -O $PLI $DEBUG $INC $RTL cent_top_tb.v
