#!/bin/bash

z80asm -v -b -s os/CPM22.z80 bios/cpm_bios_emudrive.z80
mv os/CPM22.bin .
#cat os/CPM22.sym bios/cpm_bios_emudrive.sym > ./all.sym
#z88dk-dis -m8080 -x os/CPM22.sym -o 0xdc00 -s 0xdc00 CPM22.bin > CPM22.dis
