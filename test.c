//
// Created by Steven on 18-Jan-18.
//

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "mmap.h"
#include "cpu.h"
#include "vm.h"
#include "examples/hardware/drive/drive.h"
#include "examples/hardware/term/term.h"

/*
 *  RAW MODE TEST
 *
 *
 */

//#include "examples/hardware/uart/uart.h"

int test_uart(void) {
    struct termios t;
    cpu_t cpu;
    if(term_rawmode(&t)) {
        return 0;
    }
    vm_init(&cpu);

    if(vm_loadrom_file(&cpu, 0, 0x800, "prog.bin")) {
        printf("Error: Could not open file\n");
        goto exit;
    }

    vm_loadram(&cpu, 0x8000, 0x8000, NULL);
    vm_run(&cpu);
    //vm_debug(&cpu);

exit:
    vm_destroy(&cpu);
    term_restore(&t);
    return 0;
}

/*
 *  DRIVE TEST
 *
 *
 */

int test_drive(void) {
    const char *filename = "../examples/cpm/testdisk.img";
    struct drive_t drive;
    mmap_t mmap;

    mmap_init(&mmap);
    drive_init(&drive);
    drive_load(&drive, filename, 1);
    memspace_t drivespace = DRIVE_MEMSPACE(drive, 0x0000);
    mmap_add(&mmap, &drivespace);
    mmap_print(&mmap);

    printf("\n\nWriting Sector 0, Head 0, Cylinder 0\n\n");

    mmap_write(&mmap, DRIVE_POS_DATA, DRIVE_CMD_WRITE);
    mmap_write(&mmap, DRIVE_POS_DATA, 0);
    mmap_write(&mmap, DRIVE_POS_DATA, 0);
    mmap_write(&mmap, DRIVE_POS_DATA, 1);

    for(int i = 0; i < DRIVE_SIZE_SECTOR; ++i) {
        mmap_write(&mmap, DRIVE_POS_DATA, 0xFF - i);
    }

    printf("\nResult: %02hhx\n", mmap_read(&mmap, DRIVE_POS_DATA));
    printf("\n\nReading Sector 0, Head 0, Cylinder 0\n\n");

    mmap_write(&mmap, DRIVE_POS_DATA, DRIVE_CMD_READ);
    mmap_write(&mmap, DRIVE_POS_DATA, 0);
    mmap_write(&mmap, DRIVE_POS_DATA, 0);
    mmap_write(&mmap, DRIVE_POS_DATA, 1);

    for(int i = 0; i < DRIVE_SIZE_SECTOR; ++i) {
        uint8_t ch = mmap_read(&mmap, DRIVE_POS_DATA);
        if(i % 16 == 0) {
            putchar('\n');
        }
        printf("%02hhx ", ch);
    }

    printf("\nResult: %02hhx\n", mmap_read(&mmap, DRIVE_POS_DATA));

    drive_eject(&drive);
    mmap_destroy(&mmap);

    return 0;
}

/*
 *  CPM TEST W/ DRIVE
 *
 *
 */

int test_cpm(void) {
    cpu_t cpu;
    struct drive_t drive;
    struct term_t term;
    uint8_t cpm[0x1C00];
    uint8_t *ram;
    uint8_t jmp[3] = {0xc3, 0x00, 0xf2};
/*
    FILE *m80bin = fopen("/Users/Steven/Downloads/m80.com", "rb");
    if(!m80bin) {
        exit(EXIT_FAILURE);
    }
    fseek(m80bin, 0, SEEK_END);
    long m80size = ftell(m80bin);
    rewind(m80bin);
    uint8_t m80[m80size];
    fread(m80, m80size, 1, m80bin);
*/

    //Load CP/M
    FILE *cpmbin = fopen("../examples/cpm/CPM22.bin", "rb");
    if(!cpmbin) {
        fprintf(stderr, "Error opening file.");
        exit(EXIT_FAILURE);
    }

    fseek(cpmbin, 0, SEEK_END);
    long b = ftell(cpmbin);
    rewind(cpmbin);
    fread(cpm, b, 1, cpmbin);
    fclose(cpmbin);
    memset(cpm + b, 0, 0x1C00L - b);

    //Load ram
    ram = malloc(62 * 1024);
    if(!ram) {
        fprintf(stderr, "Error allocating memory");
        exit(EXIT_FAILURE);
    }

    //Init Drive
    drive_init(&drive);
    if(drive_load(&drive, "../examples/cpm/cpmdisk.img", 1) == -1) {
        fprintf(stderr, "Error opening disk image");
        exit(EXIT_FAILURE);
    }
    term_init(&term, "term_logfile.txt");

    //Preload CP/M into Memory
    //Todo: Write bootoader that reads CP/M from drive
    memset(ram, 0, 62 * 1024);
    memcpy(ram, jmp, 3);
//    memcpy(ram + 0x100, m80, m80size);
    memcpy(ram + 0xDC00, cpm, 0x1C00);

    //Create Memory Spaces
    memspace_t drivespace = DRIVE_MEMSPACE(drive, 0xffe0);
    memspace_t termspace = TERM_MEMSPACE(term, 0);

    //Start emulation
    vm_init(&cpu);
    vm_loadram(&cpu, 0x0000, 62 * 1024, ram);
    vm_loadio(&cpu, &termspace);
    vm_loadmemio(&cpu, &drivespace);
    mmap_print(&cpu.memory);
    mmap_print(&cpu.io);

    //Change terminal settings
    struct termios orig;
    term_rawmode(&orig);
//    setvbuf(stdout, NULL, _IONBF, 0);

    vm_run(&cpu);
    //vm_debug(&cpu);

    vm_destroy(&cpu);
    drive_eject(&drive);
    term_close(&term);
    term_restore(&orig);
    return 0;
}

int test_con(void) {
    cpu_t cpu;

    vm_init(&cpu);
    if(vm_loadrom_file(&cpu, 0, -1, "../examples/program/contest/contest.bin") == -1) {
        fprintf(stderr, "Could not open contest file\n");
        exit(EXIT_FAILURE);
    }

    struct term_t term;
    term_init(&term, "term_logfile.txt");
    memspace_t termspace = TERM_MEMSPACE(term, 0);
    vm_loadio(&cpu, &termspace);

    struct termios orig;
    term_rawmode(&orig);
//    setvbuf(stdout, NULL, _IONBF, 0);

    mmap_print(&cpu.memory);
    mmap_print(&cpu.io);

    vm_run(&cpu);
    vm_destroy(&cpu);
    term_restore(&orig);
    return 0;
}