cmake_minimum_required(VERSION 3.8)
project(vm8080)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c vm.c vm.h vminst.c vminst.h cpu.h mmap.c mmap.h mmap.c mmap.h examples/hardware/mul/mul.c examples/hardware/mul/mul.h main.c bitset.h examples/hardware/drive/drive.c examples/hardware/drive/drive.h test.c test.h)
add_executable(vm2 ${SOURCE_FILES})

#add_custom_command(TARGET vm2 PRE_BUILD COMMAND /bin/bash -c ./makebin.sh)
#add_custom_target(prog.bin DEPENDS prog.asm)

# examples/hardware/i8251/i8251.c examples/hardware/i8251/i8251.h