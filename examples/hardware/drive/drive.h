//
// Created by Steven McLeod on 2018-01-11.
//

#ifndef VM2_DRIVE_H
#define VM2_DRIVE_H

#include <stdio.h>
#include <stdint.h>

#define DRIVE_POS_STAT  0
#define DRIVE_POS_DATA  1
#define DRIVE_POS_DMAL  2
#define DRIVE_POS_DMAH  3
#define DRIVE_BUF_SIZE  4

#define DRIVE_CMD_NOP       0
#define DRIVE_CMD_READ      1
#define DRIVE_CMD_WRITE     2
#define DRIVE_CMD_READTRACK 3
#define DRIVE_CMD_FORMAT    4
#define DRIVE_CMD_SCANEQ    5
#define DRIVE_CMD_SCANLE    6
#define DRIVE_CMD_SCANGE    7
#define DRIVE_CMD_SEEK      8
#define DRIVE_CMD_SETTING   9
#define DRIVE_CMD_INFO      10

#define DRIVE_STAT_DSKRDY   (1 << 0)
#define DRIVE_STAT_BUSY     (1 << 4)
#define DRIVE_STAT_DIO      (1 << 6)

#define DRIVE_RES_BADCMD    (1 << 0)
#define DRIVE_RES_BADPARAM  (1 << 1)
#define DRIVE_RES_WRITEPROT (1 << 2)

//Based on 8-inch IBM PC Compatible Single Density Single Sided 250.25KB Disk (SDSS)
#define DRIVE_SECTOR_SIZE       128
#define DRIVE_SECTOR_COUNT      26
#define DRIVE_HEAD_COUNT        1
#define DRIVE_CYLINDER_COUNT    77


#define DRIVE_MEMSPACE(obj, origin) {&drive_rd, &drive_wr, &(obj), &(obj), (obj).regs, (origin), DRIVE_BUF_SIZE, MMAP_RDWR}

struct drive_t {
    /* Implementation */
    FILE *floppy;         // Data File
    uint8_t sector_buffer[DRIVE_SECTOR_SIZE];
    int cnt;            // Index into buffer
    int state;
    uint8_t writemode;

    /* Internal Registers */
    uint8_t status;
    uint8_t cmd;
    uint8_t cylinder;
    uint8_t head;
    uint8_t sector;
    uint8_t qty;
    uint8_t result;
    uint8_t settings;
    uint8_t info;
    uint16_t dma_addr;

    /* IO Regs */
    uint8_t regs[DRIVE_BUF_SIZE];
};

int drive_init(struct drive_t *obj);//, mmap_t *memory);
int drive_reset(struct drive_t *obj);
int drive_load(struct drive_t *obj, const char *filename, uint8_t writemode);
int drive_eject(struct drive_t *obj);

void drive_rd(uint16_t pos, void *args);
void drive_wr(uint16_t pos, void *args);

#endif //VM2_DRIVE_H
