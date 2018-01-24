//
// Created by Steven McLeod on 2018-01-11.
//

#include "drive.h"
#include <string.h>
#include <stdbool.h>
#include "../../../bitset.h"

// Layers:
// 0: Interfaces with file
// 1: Updates State Machine and Byte Counters
// 2: Register Decoder

// Disk is read in this fashion:
// A sector contains 128 bytes.
// After all 26 sectors in a track are read, the head pos increments and the sector pos returns to 0.
// After all 1 heads are read, the cylinder pos increments and the head pos returns to 0.

enum {
    SCAN_LE,
    SCAN_EQ,
    SCAN_GE,
};

enum {
    STATE_WAIT=0,       // Wait for command         (ALL)
    STATE_GETC,         // Get Cylinder Pos         (seek)
    STATE_GETH,         // Get Head Pos             (All Disk rw, format, scan)
    STATE_GETS,         // Get Sector Pos           (All Disk rw, format, scan)
    STATE_GETQ,         // Get Qty of Sectors       (All Disk rw, format, scan)
    STATE_GETB,         // Get Byte                 (format, setting)
    STATE_EXEC,         // Executing until cnt = 0  (Non DMA)
    STATE_RESULT,       // Returning Result         (ALL)
};

static void drive_sm_rd(struct drive_t *obj);
static void drive_sm_wr(struct drive_t *obj);
static bool drive_validcmd(uint8_t cmd);
static void drive_runcmd(struct drive_t *obj);
static void drive_readfile(struct drive_t *obj);
static void drive_writefile(struct drive_t *obj);
static void drive_scan(struct drive_t *obj, int scanmode);
//static void drive_seek(struct drive_t *obj);
static void drive_set_filepos(struct drive_t *obj);

int drive_init(struct drive_t *obj) {//, mmap_t *memory) {
    int res = drive_reset(obj);
    //obj->memory = memory;
    return res;
}

int drive_reset(struct drive_t *obj) {
    memset(obj, 0, sizeof(struct drive_t));
    return 0;
}

int drive_load(struct drive_t *obj, const char *filename, uint8_t writemode) {
    if(obj->floppy) {
        drive_eject(obj);
    }

    char *rw = writemode ? "rb+" : "rb";
    obj->writemode = writemode;
    obj->floppy = fopen(filename, rw);
    if(!obj->floppy) {
        return -1;
    }

    BITS_SET(obj->status, DRIVE_STAT_DSKRDY);
    return 0;
}

int drive_eject(struct drive_t *obj) {
    int res = -1;
    if(obj->floppy) {
        int res = fclose(obj->floppy);
        obj->floppy = NULL;
    }

    BITS_RESET(obj->status, DRIVE_STAT_DSKRDY);
    return res;
}

void drive_rd(uint16_t pos, void *args) {
    struct drive_t *obj = args;

    switch(pos) {
        case DRIVE_POS_STAT:
            obj->regs[DRIVE_POS_STAT] = obj->status;
            break;

        case DRIVE_POS_DATA:
            drive_sm_rd(obj);
            break;

        case DRIVE_POS_DMAL:
            obj->regs[DRIVE_POS_DMAL] = obj->dma_addr & 0x00FF;
            break;

        case DRIVE_POS_DMAH:
            obj->regs[DRIVE_POS_DMAH] = (obj->dma_addr & 0xFF00) >> 8;
            break;
    }
}

void drive_wr(uint16_t pos, void *args) {
    struct drive_t *obj = args;

    switch(pos) {
        case DRIVE_POS_STAT:
            break;

        case DRIVE_POS_DATA:
            drive_sm_wr(obj);
            break;

        case DRIVE_POS_DMAL:
            obj->dma_addr &= obj->regs[DRIVE_POS_DMAL];
            break;

        case DRIVE_POS_DMAH:
            obj->dma_addr &= obj->regs[DRIVE_POS_DMAH] << 8;
            break;
    }
}

/*
 *  Statics
 *
 */

void drive_sm_rd(struct drive_t *obj) {
    switch(obj->state) {
        //Continuous execution until qty == 0
        case STATE_EXEC:
            drive_runcmd(obj);
            if(obj->qty == 0) {
                BITS_RESET(obj->status, DRIVE_STAT_BUSY);
                obj->state = STATE_RESULT;
            }

            break;

        //Put the result of the last command
        case STATE_RESULT:
            obj->regs[DRIVE_POS_DATA] = obj->result;
            obj->state = STATE_WAIT;
            break;

        default:
            obj->regs[DRIVE_POS_DATA] = 0;
    }
}

void drive_sm_wr(struct drive_t *obj) {
    uint8_t datareg = obj->regs[DRIVE_POS_DATA];

    switch(obj->state) {
        // Check if a valid command was input.
        case STATE_WAIT:
            if(!drive_validcmd(datareg)) {
                BITS_SET(obj->result, DRIVE_RES_BADCMD);
                obj->state = STATE_RESULT;
            } else {
                obj->cmd = datareg;
                BITS_SET(obj->status, DRIVE_STAT_BUSY);

                if(datareg == DRIVE_CMD_SEEK) {
                    obj->state = STATE_GETC;
                } else {
                    obj->state = STATE_GETH;
                }
            }

            break;

            // Get the cylinder position
        case STATE_GETC:
            // Check data validity
            if(datareg >= DRIVE_QTY_CYLINDER) {
                BITS_SET(obj->result, DRIVE_RES_BADPARAM);
            } else {
                obj->cylinder = datareg;
            }
            obj->state = STATE_RESULT;
            break;

            // Get the head position
        case STATE_GETH:
            obj->head = datareg;
            obj->state = STATE_GETS;
            break;

            // Get the sector position
        case STATE_GETS:
            obj->sector = datareg;
            obj->state = STATE_GETQ;
            break;

            // Get the number of sectors to read
        case STATE_GETQ:
            obj->qty = datareg;

            // Check data validity
            if(obj->sector >= DRIVE_QTY_SECTOR) {
                BITS_SET(obj->result, DRIVE_RES_BADPARAM);
                obj->state = STATE_RESULT;
            } else if(obj->head >= DRIVE_QTY_HEAD) {
                BITS_SET(obj->result, DRIVE_RES_BADPARAM);
                obj->state = STATE_RESULT;
            } else if(obj->cmd == DRIVE_CMD_WRITE && !obj->writemode) {
                BITS_SET(obj->result, DRIVE_RES_WRITEPROT);
                obj->state = STATE_RESULT;
            } else {
                obj->state = STATE_EXEC;
            }

            break;

            // Continuous execution until qty == 0
        case STATE_EXEC:
            drive_runcmd(obj);
            if(obj->qty == 0) {
                BITS_RESET(obj->status, DRIVE_STAT_BUSY);
                obj->state = STATE_RESULT;
            }

            break;

        case STATE_GETB:
            break;

        case STATE_RESULT:
            break;
    }
}

bool drive_validcmd(uint8_t cmd) {
    return  //cmd == DRIVE_CMD_NOP ||
        cmd == DRIVE_CMD_READ ||
        cmd == DRIVE_CMD_WRITE ||
        //cmd == DRIVE_CMD_READTRACK ||
        //cmd == DRIVE_CMD_FORMAT ||
        //cmd == DRIVE_CMD_SCANEQ ||
        //cmd == DRIVE_CMD_SCANLE ||
        //cmd == DRIVE_CMD_SCANGE ||
        cmd == DRIVE_CMD_SEEK //||
        //cmd == DRIVE_CMD_SETTING ||
        //cmd == DRIVE_CMD_INFO
        ;
}

void drive_runcmd(struct drive_t *obj) {
    switch(obj->cmd) {
        case DRIVE_CMD_READ:        drive_readfile(obj); break;
        case DRIVE_CMD_WRITE:       drive_writefile(obj); break;
        case DRIVE_CMD_READTRACK:   obj->qty = DRIVE_QTY_SECTOR; drive_readfile(obj); break;
        case DRIVE_CMD_FORMAT:      break;//drive_format(obj); break;
        case DRIVE_CMD_SCANEQ:      break;//drive_scan(obj, SCAN_EQ); break;
        case DRIVE_CMD_SCANLE:      break;//drive_scan(obj, SCAN_LE); break;
        case DRIVE_CMD_SCANGE:      break;//drive_scan(obj, SCAN_GE); break;
        case DRIVE_CMD_SEEK:        break;//drive_seek(obj); break;
        case DRIVE_CMD_SETTING:     /* TODO */ break;
        case DRIVE_CMD_INFO:        /* TODO */ break;
        default:                    ;
    }
}

void drive_readfile(struct drive_t *obj) {
    if(obj->cnt == 0) {
        drive_set_filepos(obj);
        fread(obj->sector_buffer, DRIVE_SIZE_SECTOR, 1, obj->floppy);
    }

    obj->regs[DRIVE_POS_DATA] = obj->sector_buffer[obj->cnt];
    if(obj->cnt == DRIVE_SIZE_SECTOR - 1) {
        obj->cnt = 0;
        ++obj->sector;

        if(obj->sector == DRIVE_QTY_SECTOR) {
            obj->sector = 0;
            obj->qty = 0;
        } else {
            --obj->qty;
        }
    } else {
        ++obj->cnt;
    }
}

void drive_writefile(struct drive_t *obj) {
    obj->sector_buffer[obj->cnt] = obj->regs[DRIVE_POS_DATA];
    if(obj->cnt == DRIVE_SIZE_SECTOR - 1) {
        drive_set_filepos(obj);
        fwrite(obj->sector_buffer, DRIVE_SIZE_SECTOR, 1, obj->floppy);
        obj->cnt = 0;
        ++obj->sector;

        if(obj->sector == DRIVE_QTY_SECTOR) {
            obj->sector = 0;
            obj->qty = 0;
        } else {
            --obj->qty;
        }
    } else {
        ++obj->cnt;
    }
}

void drive_format(struct drive_t *obj);
void drive_scan(struct drive_t *obj, int scanmode);

//void drive_seek(struct drive_t *obj) {
//    long filepos = (long) obj->cylinder * DRIVE_SIZE_SECTOR * DRIVE_QTY_SECTOR * DRIVE_QTY_HEAD;
//    fseek(obj->floppy, filepos, SEEK_SET);
//}

// Todo: What happens if seek is outside file range?
void drive_set_filepos(struct drive_t *obj) {
    long filepos =
        (long) (obj->sector * DRIVE_SIZE_SECTOR) +
        (long) (obj->head * DRIVE_SIZE_HEAD) +
        (long) (obj->cylinder * DRIVE_SIZE_CYLINDER);

    fseek(obj->floppy, filepos, SEEK_SET);
}