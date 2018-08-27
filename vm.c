//
// Created by Steven McLeod on 2017-11-07.
//

#include "vm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "vminst.h"

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/*
 *  Error Codes:
 *      1:  Bad Opcode
 *      2:  Invalid Addressing Mode
 *      3:  Unexpected EOF
 *      4:  Invalid RST Number
 *      5:  Invalid Condition Mode
 */

struct {
    uint16_t bps[64];
    int size;
} breakpoints;

static void add_bp(uint16_t addr) {
    breakpoints.bps[breakpoints.size] = addr;
    ++breakpoints.size;
}

int vm_init(cpu_t *obj) {
    memset(obj, 0, sizeof(cpu_t));
    obj->reg_f = CPU_FLAG_DEFAULT;
    mmap_init(&obj->memory);
    mmap_init(&obj->io);
    return 0;
}

int vm_loadrom(cpu_t *obj, uint16_t origin, uint16_t len, uint8_t *ptr) {
    memspace_t space = {0, 0, 0, 0, ptr, origin, len, MMAP_READ, "ROM"};
    return mmap_add(&obj->memory, &space);
}

int vm_loadram(cpu_t *obj, uint16_t origin, uint16_t len, uint8_t *ptr) {
    uint8_t flags = MMAP_RDWR | (ptr ? 0 : MMAP_ALLOCFREE);
    memspace_t space = {0, 0, 0, 0, ptr, origin, len, flags, "RAM"};
    return mmap_add(&obj->memory, &space);
}

int vm_loadrom_file(cpu_t *obj, uint16_t origin, uint16_t len, const char *fn) {
    int retcode;

    FILE *f = fopen(fn, "rb");
    if(!f) {
        retcode = -1;
        goto exit_return;
    }

    fseek(f, 0, SEEK_END);
    long _flen = ftell(f);
    uint16_t flen = (uint16_t) (_flen > UINT16_MAX ? UINT16_MAX : _flen);
    rewind(f);

    //Buffer size is len specified
    uint16_t readsize = min(len, flen);
    uint8_t *ptr = malloc(len);
    if(!ptr) {
        retcode = -1;
        goto exit_file;
    }
    fread(ptr, readsize, 1, f);

    //If the file did not completely fill the buffer, set to 0
    if(len > flen) {
        memset(ptr + flen, 0, len - flen);
    }

    memspace_t space = {0, 0, 0, 0, ptr, origin, len, MMAP_READ | MMAP_FREE, "FILEROM"};
    retcode = mmap_add(&obj->memory, &space);

exit_file:
    fclose(f);

exit_return:
    return retcode;
}

int vm_loadio(cpu_t *obj, const memspace_t *space) {
    return mmap_add(&obj->io, space);
}

unsigned vm_loadio_arr(cpu_t *obj, const memspace_t *space, unsigned len) {
    for(unsigned i = 0; i < len; ++i) {
        if(mmap_add(&obj->io, &space[i]) == -1)
            return i;
    }

    return len;
}

int vm_loadmemio(cpu_t *obj, const memspace_t *space) {
    return mmap_add(&obj->memory, space);
}

unsigned vm_loadmemio_arr(cpu_t *obj, const memspace_t *space, unsigned len) {
    for(unsigned i = 0; i < len; ++i) {
        if(mmap_add(&obj->memory, &space[i]) == -1)
            return i;
    }

    return len;
}

int vm_reset(cpu_t *obj) {
    memset(obj, 0, CPU_CORE_SIZE);
    obj->reg_f = CPU_FLAG_DEFAULT;
    return 0;
}

int vm_destroy(cpu_t *obj) {
    mmap_destroy(&obj->memory);
    mmap_destroy(&obj->io);
    return 0;
}

int vm_run(cpu_t *obj) {
    while(!obj->halt) {
        int retcode = vm_singlestep(obj);
        if(retcode) {
            return retcode;
        }
    }

    return 0;
}

int vm_debug(cpu_t *obj) {
    add_bp(0);
    //add_bp(0xdf98);
    //add_bp(0xdff8);

    add_bp(0xf083);         // RESDSK

    // BITMAP2

    /*
    add_bp(0xf0ec);         // FCREATE
    add_bp(0xed3d);         // GETEMPTY: RET Z
    //add_bp(0xed24);         // GETEMPTY
    add_bp(0xeb2d);         // FINDNXT
    add_bp(0xeb93);         // FINDNXT: Success
    add_bp(0xeb94);         // FNDNXT6
    add_bp(0xe9d6);         // DIRREAD
     */

    //add_bp(0xead2);         // CHECKDIR E9C0

    while(!obj->halt) {
        char *carrystr = (CPU_BIT_TEST(obj, CPU_CARRY_POS)) ? " C" : "NC";
        char *zerostr = (CPU_BIT_TEST(obj, CPU_ZERO_POS)) ? " Z" : "NZ";
        char *parstr = (CPU_BIT_TEST(obj, CPU_PARITY_POS)) ? "PE" : "PO";
        char *negstr = (CPU_BIT_TEST(obj, CPU_SIGN_POS)) ? " N" : " P";
        char *halfstr = (CPU_BIT_TEST(obj, CPU_AC_POS)) ? " H" : "NH";

        printf(
                "A=%02hhx B=%02hhx C=%02hhx D=%02hhx E=%02hhx H=%02hhx L=%02hhx\n"
                "PC=%04hx   SP=%04hx   %s %s %s %s %s\n"
                "Opcode: %02hhx (, %02hhx, %02hhx)\n\n",
                obj->reg_a, obj->reg_b, obj->reg_c, obj->reg_d, obj->reg_e, obj->reg_h, obj->reg_l,
                obj->reg_pc, obj->reg_sp, carrystr, zerostr, parstr, negstr, halfstr,
                mmap_read_noexec(&obj->memory, obj->reg_pc), mmap_read_noexec(&obj->memory, obj->reg_pc + 1), mmap_read_noexec(&obj->memory, obj->reg_pc + 2)
        );

        for(int i = 0; i < breakpoints.size; ++i) {
            if(breakpoints.bps[i] == obj->reg_pc) {
                break;
            }
        }

        int retcode = vm_singlestep(obj);

        if(retcode) {
            return retcode;
        }
    }

    return 0;
}

/*
int vm_printinfo(cpu_t *obj) {
    printf(
            "Instructions: %u\n"
            "Reg-Reads:    %u\n"
            "Reg-Write:    %u\n"
            "Mem-Reads:    %u\n"
            "Mem-Write:    %u\n"
            "IOR-Reads:    %u\n"
            "IOR-Write:    %u\n",
            obj->stats.instruction, obj->stats.regread, obj->stats.regwrite,
            obj->stats.memread, obj->stats.memwrite, obj->stats.ioread, obj->stats.iowrite
    );
}
 */

int vm_singlestep(cpu_t *obj) {
    uint8_t op = mmap_read(&obj->memory, obj->reg_pc);
    ++obj->reg_pc;

    switch(op) {
        case 0x00: vm_nop(obj); break;
        case 0x01: vm_lxi(obj, OPX_BC); break;
        case 0x02: vm_stax(obj, OPX_BC); break;
        case 0x03: vm_inx(obj, OPX_BC); break;
        case 0x04: vm_inr(obj, OP_B); break;
        case 0x05: vm_dcr(obj, OP_B); break;
        case 0x06: vm_mov(obj, OP_B, OP_IMMEDIATE); break;
        case 0x07: vm_rlc(obj); break;
        case 0x08: vm_nop(obj); break;
        case 0x09: vm_dad(obj, OPX_BC); break;
        case 0x0a: vm_ldax(obj, OPX_BC); break;
        case 0x0b: vm_dcx(obj, OPX_BC); break;
        case 0x0c: vm_inr(obj, OP_C); break;
        case 0x0d: vm_dcr(obj, OP_C); break;
        case 0x0e: vm_mov(obj, OP_C, OP_IMMEDIATE); break;
        case 0x0f: vm_rrc(obj); break;

        case 0x10: vm_nop(obj); break;
        case 0x11: vm_lxi(obj, OPX_DE); break;
        case 0x12: vm_stax(obj, OPX_DE); break;
        case 0x13: vm_inx(obj, OPX_DE); break;
        case 0x14: vm_inr(obj, OP_D); break;
        case 0x15: vm_dcr(obj, OP_D); break;
        case 0x16: vm_mov(obj, OP_D, OP_IMMEDIATE); break;
        case 0x17: vm_ral(obj); break;
        case 0x18: vm_nop(obj); break;
        case 0x19: vm_dad(obj, OPX_DE); break;
        case 0x1a: vm_ldax(obj, OPX_DE); break;
        case 0x1b: vm_dcx(obj, OPX_DE); break;
        case 0x1c: vm_inr(obj, OP_E); break;
        case 0x1d: vm_dcr(obj, OP_E); break;
        case 0x1e: vm_mov(obj, OP_E, OP_IMMEDIATE); break;
        case 0x1f: vm_rar(obj); break;

        case 0x20: vm_nop(obj); break;
        case 0x21: vm_lxi(obj, OPX_HL); break;
        case 0x22: vm_shld(obj); break;
        case 0x23: vm_inx(obj, OPX_HL); break;
        case 0x24: vm_inr(obj, OP_H); break;
        case 0x25: vm_dcr(obj, OP_H); break;
        case 0x26: vm_mov(obj, OP_H, OP_IMMEDIATE); break;
        case 0x27: vm_daa(obj); break;
        case 0x28: vm_nop(obj); break;
        case 0x29: vm_dad(obj, OPX_HL); break;
        case 0x2a: vm_lhld(obj); break;
        case 0x2b: vm_dcx(obj, OPX_HL); break;
        case 0x2c: vm_inr(obj, OP_L); break;
        case 0x2d: vm_dcr(obj, OP_L); break;
        case 0x2e: vm_mov(obj, OP_L, OP_IMMEDIATE); break;
        case 0x2f: vm_cma(obj); break;

        case 0x30: vm_nop(obj); break;
        case 0x31: vm_lxi(obj, OPX_SP); break;
        case 0x32: vm_sta(obj); break;
        case 0x33: vm_inx(obj, OPX_SP); break;
        case 0x34: vm_inr(obj, OP_M); break;
        case 0x35: vm_dcr(obj, OP_M); break;
        case 0x36: vm_mov(obj, OP_M, OP_IMMEDIATE); break;
        case 0x37: vm_stc(obj); break;
        case 0x38: vm_nop(obj); break;
        case 0x39: vm_dad(obj, OPX_SP); break;
        case 0x3a: vm_lda(obj); break;
        case 0x3b: vm_dcx(obj, OPX_SP); break;
        case 0x3c: vm_inr(obj, OP_A); break;
        case 0x3d: vm_dcr(obj, OP_A); break;
        case 0x3e: vm_mov(obj, OP_A, OP_IMMEDIATE); break;
        case 0x3f: vm_cmc(obj); break;

        case 0x40: vm_mov(obj, OP_B, OP_B); break;
        case 0x41: vm_mov(obj, OP_B, OP_C); break;
        case 0x42: vm_mov(obj, OP_B, OP_D); break;
        case 0x43: vm_mov(obj, OP_B, OP_E); break;
        case 0x44: vm_mov(obj, OP_B, OP_H); break;
        case 0x45: vm_mov(obj, OP_B, OP_L); break;
        case 0x46: vm_mov(obj, OP_B, OP_M); break;
        case 0x47: vm_mov(obj, OP_B, OP_A); break;
        case 0x48: vm_mov(obj, OP_C, OP_B); break;
        case 0x49: vm_mov(obj, OP_C, OP_C); break;
        case 0x4a: vm_mov(obj, OP_C, OP_D); break;
        case 0x4b: vm_mov(obj, OP_C, OP_E); break;
        case 0x4c: vm_mov(obj, OP_C, OP_H); break;
        case 0x4d: vm_mov(obj, OP_C, OP_L); break;
        case 0x4e: vm_mov(obj, OP_C, OP_M); break;
        case 0x4f: vm_mov(obj, OP_C, OP_A); break;

        case 0x50: vm_mov(obj, OP_D, OP_B); break;
        case 0x51: vm_mov(obj, OP_D, OP_C); break;
        case 0x52: vm_mov(obj, OP_D, OP_D); break;
        case 0x53: vm_mov(obj, OP_D, OP_E); break;
        case 0x54: vm_mov(obj, OP_D, OP_H); break;
        case 0x55: vm_mov(obj, OP_D, OP_L); break;
        case 0x56: vm_mov(obj, OP_D, OP_M); break;
        case 0x57: vm_mov(obj, OP_D, OP_A); break;
        case 0x58: vm_mov(obj, OP_E, OP_B); break;
        case 0x59: vm_mov(obj, OP_E, OP_C); break;
        case 0x5a: vm_mov(obj, OP_E, OP_D); break;
        case 0x5b: vm_mov(obj, OP_E, OP_E); break;
        case 0x5c: vm_mov(obj, OP_E, OP_H); break;
        case 0x5d: vm_mov(obj, OP_E, OP_L); break;
        case 0x5e: vm_mov(obj, OP_E, OP_M); break;
        case 0x5f: vm_mov(obj, OP_E, OP_A); break;

        case 0x60: vm_mov(obj, OP_H, OP_B); break;
        case 0x61: vm_mov(obj, OP_H, OP_C); break;
        case 0x62: vm_mov(obj, OP_H, OP_D); break;
        case 0x63: vm_mov(obj, OP_H, OP_E); break;
        case 0x64: vm_mov(obj, OP_H, OP_H); break;
        case 0x65: vm_mov(obj, OP_H, OP_L); break;
        case 0x66: vm_mov(obj, OP_H, OP_M); break;
        case 0x67: vm_mov(obj, OP_H, OP_A); break;
        case 0x68: vm_mov(obj, OP_L, OP_B); break;
        case 0x69: vm_mov(obj, OP_L, OP_C); break;
        case 0x6a: vm_mov(obj, OP_L, OP_D); break;
        case 0x6b: vm_mov(obj, OP_L, OP_E); break;
        case 0x6c: vm_mov(obj, OP_L, OP_H); break;
        case 0x6d: vm_mov(obj, OP_L, OP_L); break;
        case 0x6e: vm_mov(obj, OP_L, OP_M); break;
        case 0x6f: vm_mov(obj, OP_L, OP_A); break;

        case 0x70: vm_mov(obj, OP_M, OP_B); break;
        case 0x71: vm_mov(obj, OP_M, OP_C); break;
        case 0x72: vm_mov(obj, OP_M, OP_D); break;
        case 0x73: vm_mov(obj, OP_M, OP_E); break;
        case 0x74: vm_mov(obj, OP_M, OP_H); break;
        case 0x75: vm_mov(obj, OP_M, OP_L); break;
        case 0x76: vm_hlt(obj); break;
        case 0x77: vm_mov(obj, OP_M, OP_A); break;
        case 0x78: vm_mov(obj, OP_A, OP_B); break;
        case 0x79: vm_mov(obj, OP_A, OP_C); break;
        case 0x7a: vm_mov(obj, OP_A, OP_D); break;
        case 0x7b: vm_mov(obj, OP_A, OP_E); break;
        case 0x7c: vm_mov(obj, OP_A, OP_H); break;
        case 0x7d: vm_mov(obj, OP_A, OP_L); break;
        case 0x7e: vm_mov(obj, OP_A, OP_M); break;
        case 0x7f: vm_mov(obj, OP_A, OP_A); break;

        case 0x80: vm_add(obj, OP_B); break;
        case 0x81: vm_add(obj, OP_C); break;
        case 0x82: vm_add(obj, OP_D); break;
        case 0x83: vm_add(obj, OP_E); break;
        case 0x84: vm_add(obj, OP_H); break;
        case 0x85: vm_add(obj, OP_L); break;
        case 0x86: vm_add(obj, OP_M); break;
        case 0x87: vm_add(obj, OP_A); break;
        case 0x88: vm_adc(obj, OP_B); break;
        case 0x89: vm_adc(obj, OP_C); break;
        case 0x8a: vm_adc(obj, OP_D); break;
        case 0x8b: vm_adc(obj, OP_E); break;
        case 0x8c: vm_adc(obj, OP_H); break;
        case 0x8d: vm_adc(obj, OP_L); break;
        case 0x8e: vm_adc(obj, OP_M); break;
        case 0x8f: vm_adc(obj, OP_A); break;

        case 0x90: vm_sub(obj, OP_B); break;
        case 0x91: vm_sub(obj, OP_C); break;
        case 0x92: vm_sub(obj, OP_D); break;
        case 0x93: vm_sub(obj, OP_E); break;
        case 0x94: vm_sub(obj, OP_H); break;
        case 0x95: vm_sub(obj, OP_L); break;
        case 0x96: vm_sub(obj, OP_M); break;
        case 0x97: vm_sub(obj, OP_A); break;
        case 0x98: vm_sbb(obj, OP_B); break;
        case 0x99: vm_sbb(obj, OP_C); break;
        case 0x9a: vm_sbb(obj, OP_D); break;
        case 0x9b: vm_sbb(obj, OP_E); break;
        case 0x9c: vm_sbb(obj, OP_H); break;
        case 0x9d: vm_sbb(obj, OP_L); break;
        case 0x9e: vm_sbb(obj, OP_M); break;
        case 0x9f: vm_sbb(obj, OP_A); break;

        case 0xa0: vm_ana(obj, OP_B); break;
        case 0xa1: vm_ana(obj, OP_C); break;
        case 0xa2: vm_ana(obj, OP_D); break;
        case 0xa3: vm_ana(obj, OP_E); break;
        case 0xa4: vm_ana(obj, OP_H); break;
        case 0xa5: vm_ana(obj, OP_L); break;
        case 0xa6: vm_ana(obj, OP_M); break;
        case 0xa7: vm_ana(obj, OP_A); break;
        case 0xa8: vm_xra(obj, OP_B); break;
        case 0xa9: vm_xra(obj, OP_C); break;
        case 0xaa: vm_xra(obj, OP_D); break;
        case 0xab: vm_xra(obj, OP_E); break;
        case 0xac: vm_xra(obj, OP_H); break;
        case 0xad: vm_xra(obj, OP_L); break;
        case 0xae: vm_xra(obj, OP_M); break;
        case 0xaf: vm_xra(obj, OP_A); break;

        case 0xb0: vm_ora(obj, OP_B); break;
        case 0xb1: vm_ora(obj, OP_C); break;
        case 0xb2: vm_ora(obj, OP_D); break;
        case 0xb3: vm_ora(obj, OP_E); break;
        case 0xb4: vm_ora(obj, OP_H); break;
        case 0xb5: vm_ora(obj, OP_L); break;
        case 0xb6: vm_ora(obj, OP_M); break;
        case 0xb7: vm_ora(obj, OP_A); break;
        case 0xb8: vm_cmp(obj, OP_B); break;
        case 0xb9: vm_cmp(obj, OP_C); break;
        case 0xba: vm_cmp(obj, OP_D); break;
        case 0xbb: vm_cmp(obj, OP_E); break;
        case 0xbc: vm_cmp(obj, OP_H); break;
        case 0xbd: vm_cmp(obj, OP_L); break;
        case 0xbe: vm_cmp(obj, OP_M); break;
        case 0xbf: vm_cmp(obj, OP_A); break;

        case 0xc0: vm_rcond(obj, COND_NZ); break;
        case 0xc1: vm_pop(obj, OPX_BC); break;
        case 0xc2: vm_jcond(obj, COND_NZ); break;
        case 0xc3: vm_jmp(obj); break;
        case 0xc4: vm_ccond(obj, COND_NZ); break;
        case 0xc5: vm_push(obj, OPX_BC); break;
        case 0xc6: vm_add(obj, OP_IMMEDIATE); break;
        case 0xc7: vm_rst(obj, 0); break;
        case 0xc8: vm_rcond(obj, COND_Z); break;
        case 0xc9: vm_ret(obj); break;
        case 0xca: vm_jcond(obj, COND_Z); break;
        case 0xcb: vm_jmp(obj); break;                  //Do not use
        case 0xcc: vm_ccond(obj, COND_Z); break;
        case 0xcd: vm_call(obj); break;
        case 0xce: vm_adc(obj, OP_IMMEDIATE); break;
        case 0xcf: vm_rst(obj, 1); break;

        case 0xd0: vm_rcond(obj, COND_NC); break;
        case 0xd1: vm_pop(obj, OPX_DE); break;
        case 0xd2: vm_jcond(obj, COND_NC); break;
        case 0xd3: vm_out(obj); break;
        case 0xd4: vm_ccond(obj, COND_NC); break;
        case 0xd5: vm_push(obj, OPX_DE); break;
        case 0xd6: vm_sub(obj, OP_IMMEDIATE); break;
        case 0xd7: vm_rst(obj, 2); break;
        case 0xd8: vm_rcond(obj, COND_C); break;
        case 0xd9: vm_ret(obj); break;                  //Do not use
        case 0xda: vm_jcond(obj, COND_C); break;
        case 0xdb: vm_in(obj); break;
        case 0xdc: vm_ccond(obj, COND_C); break;
        case 0xdd: vm_call(obj); break;                 //Do not use
        case 0xde: vm_sbb(obj, OP_IMMEDIATE); break;
        case 0xdf: vm_rst(obj, 3); break;

        case 0xe0: vm_rcond(obj, COND_PO); break;
        case 0xe1: vm_pop(obj, OPX_HL); break;
        case 0xe2: vm_jcond(obj, COND_PO); break;
        case 0xe3: vm_xthl(obj); break;
        case 0xe4: vm_ccond(obj, COND_PO); break;
        case 0xe5: vm_push(obj, OPX_HL); break;
        case 0xe6: vm_ana(obj, OP_IMMEDIATE); break;
        case 0xe7: vm_rst(obj, 4); break;
        case 0xe8: vm_rcond(obj, COND_PE); break;
        case 0xe9: vm_pchl(obj); break;
        case 0xea: vm_jcond(obj, COND_PE); break;
        case 0xeb: vm_xchg(obj); break;
        case 0xec: vm_ccond(obj, COND_PE); break;
        case 0xed: vm_call(obj); break;                 //Do not use
        case 0xee: vm_xra(obj, OP_IMMEDIATE); break;
        case 0xef: vm_rst(obj, 5); break;

        case 0xf0: vm_rcond(obj, COND_P); break;
        case 0xf1: vm_pop(obj, OPX_PSW); break;
        case 0xf2: vm_jcond(obj, COND_P); break;
        case 0xf3: vm_di(obj); break;
        case 0xf4: vm_ccond(obj, COND_P); break;
        case 0xf5: vm_push(obj, OPX_PSW); break;
        case 0xf6: vm_ora(obj, OP_IMMEDIATE); break;
        case 0xf7: vm_rst(obj, 6); break;
        case 0xf8: vm_rcond(obj, COND_M); break;
        case 0xf9: vm_sphl(obj); break;
        case 0xfa: vm_jcond(obj, COND_M); break;
        case 0xfb: vm_ei(obj); break;
        case 0xfc: vm_ccond(obj, COND_M); break;
        case 0xfd: vm_call(obj); break;                 //Do not use
        case 0xfe: vm_cmp(obj, OP_IMMEDIATE); break;
        case 0xff: vm_rst(obj, 5); break;

        default: {
            char msg[256];
            snprintf(msg, 256, "Bad opcode: 0x%02hhx", op);
            vm_terminate(1, msg, obj);
        }
    }

    return 0;
    //return vm_cycles[op];
}