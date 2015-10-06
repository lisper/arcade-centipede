#!/bin/sh

RTL="../rtl/centipede.v ../rtl/p6502.v ../rtl/pokey.v \
     ../rtl/ram.v ../rtl/rom.v \
     ../rtl/color_ram.v ../rtl/pf_ram.v  ../rtl/pf_rom.v \
     ../rtl/vprom.v ../rtl/hs_ram.v"

INC="+incdir+../6502/orig +incdir+../rtl"
INC="+incdir+../6502/ac6502-0.6 +incdir+../rtl"

DEBUG="+define+debug=1 +define+SIMULATION=1"
#DEBUG=+define+SIMULATION=1

PLI=+loadvpi=../pli/vga/vga.so:vpi_compat_bootstrap

cver $PLI $DEBUG $INC $RTL centipede_tb.v
