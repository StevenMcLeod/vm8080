#!/usr/bin/env bash

set -e
printf "Assembling...\n"
ls | grep -P "^prog\.(?!(asm|bin)$).*$" | xargs -d"\n" rm

z80asm -b -s -l -otemp.bin -I../examples/software -L../examples/software/lib -icrt0 -istdio -istring prog.asm

if [ -f prog.bin ]; then
    rm prog.bin
fi

mv temp.bin prog.bin
z88dk-dis -o0 -m8080 -x prog.sym prog.bin > prog.dis


printf "Done.\n"