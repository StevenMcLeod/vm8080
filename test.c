//
// Created by Steven on 18-Jan-18.
//

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "mmap.h"
#include "cpu.h"
#include "vm.h"

/*
 *  UART TEST
 *
 *
 */

#include "examples/hardware/uart/uart.h"

static int init_term(struct termios *term) {
    struct termios term_new;

    if(!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: Not a tty\n");
        return -1;
    }

    if(tcgetattr(STDIN_FILENO, term)) {
        fprintf(stderr, "Error: Could not get term attr\n");
        return -1;
    }

    term_new = *term;

    term_new.c_lflag &= ~(ICANON | ECHO);
    term_new.c_cc[VTIME] = 0;
    term_new.c_cc[VMIN] = 1;

    if(tcsetattr(STDIN_FILENO, TCSANOW, &term_new)) {
        fprintf(stderr, "Error: Could not set term attr\n");
        return -1;
    }

    return 0;
}

static inline int end_term(struct termios *term) {
    tcsetattr(STDIN_FILENO, TCSANOW, term);
}

int test_uart(void) {
    struct termios t;
    cpu_t cpu;
    if(init_term(&t)) {
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
    end_term(&t);
}

/*
 *  DRIVE TEST
 *
 *
 */

#include "examples/hardware/drive/drive.h"

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

    for(int i = 0; i < DRIVE_SECTOR_SIZE; ++i) {
        mmap_write(&mmap, DRIVE_POS_DATA, 0xFF - i);
    }

    printf("\nResult: %02hhx\n", mmap_read(&mmap, DRIVE_POS_DATA));
    printf("\n\nReading Sector 0, Head 0, Cylinder 0\n\n");

    mmap_write(&mmap, DRIVE_POS_DATA, DRIVE_CMD_READ);
    mmap_write(&mmap, DRIVE_POS_DATA, 0);
    mmap_write(&mmap, DRIVE_POS_DATA, 0);
    mmap_write(&mmap, DRIVE_POS_DATA, 1);

    for(int i = 0; i < DRIVE_SECTOR_SIZE; ++i) {
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
 *  CPM TEST
 *
 *
 */

uint8_t startup[3] = {
    0xc3, 0x10, 0xf8,
};

uint8_t osbuf[0x100] = {
    0xc3, 0x00, 0xf8, 0x00, 0x00, 0xc3, 0x06, 0xe4
};

static uint8_t biosmem[16];
MEMFUNC(biosfunc, pos, params) {
    static const char *funcs[16] = {
        "WBOOT",
        "CONST",
        "CONIN",
        "CONOUT",
        "LIST",
        "PUNCH",
        "READER",
        "HOME",
        "SELDSK",
        "SETTRK",
        "SETSEC",
        "SETDMA",
        "READ",
        "WRITE",
        "PRSTAT",
        "SECTRN",
    };

    fprintf(stderr, "Error: Bios function %s called.\n", funcs[pos]);
    exit(EXIT_FAILURE);
}

static uint8_t bootmem[1] = { 0 };
MEMFUNC(bootfunc, pos, params) {
    uint8_t *ram = params;
    memcpy(ram, osbuf, 8);
}

#include "examples/hardware/drive/drive.h"

int test_cpm(void) {
    cpu_t cpu;
    struct drive_t drive;
    uint8_t cpm[0x1C00];
    uint8_t *ram;
    uint8_t jmp[3] = {0xc3, 0x00, 0xf2};

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
    if(drive_load(&drive, "../examples/cpm/formatdisk.img", 1) == -1) {
        fprintf(stderr, "Error opening disk image");
        exit(EXIT_FAILURE);
    }

    memset(ram, 0, 62 * 1024);
    memcpy(ram, startup, 3);
    memcpy(ram + 0xDC00, cpm, 0x1C00);

    //Load Bios
    memspace_t bootspace = {&bootfunc, &bootfunc, ram, ram, bootmem, 0xF810, 1, MMAP_READ};
    memspace_t drivespace = DRIVE_MEMSPACE(drive, 0xffe0);

    //Start emulation
    vm_init(&cpu);
    vm_loadram(&cpu, 0x0000, 62 * 1024, ram);
    vm_loadio(&cpu, &bootspace);
    vm_loadio(&cpu, &drivespace);
    vm_loadrom(&cpu, 0xf811, 3, jmp);
    mmap_print(&cpu.memory);

    vm_run(&cpu);
    //vm_debug(&cpu);
    vm_destroy(&cpu);

    return 0;
}