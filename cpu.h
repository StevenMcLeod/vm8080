/*
 *  cpu.h
 *  CPU data type definition, register enums, and supporting macros
 *
 *
 */

#ifndef VM8080_CPU_H
#define VM8080_CPU_H

#include <stdint.h>
#include <stddef.h>
#include "mmap.h"
#include "bitset.h"

#define EVAL_BOOL(n)        ((n)?1:0)
#define FLAG_MASK           0b11010101
#define CPU_FLAG_DEFAULT    0x02

//Flag Bit Positions
#define CPU_SIGN_POS        7
#define CPU_ZERO_POS        6
#define CPU_AC_POS          4
#define CPU_PARITY_POS      2
#define CPU_CARRY_POS       0

//Flag Bits
#define CPU_SIGN            (1 << CPU_SIGN_POS)
#define CPU_ZERO            (1 << CPU_ZERO_POS)
#define CPU_AC              (1 << CPU_AC_POS)
#define CPU_PARITY          (1 << CPU_PARITY_POS)
#define CPU_CARRY           (1 << CPU_CARRY_POS)

//Maskable Bit Macros
/*
#define CPU_MASK_SET(obj, mask)         (obj)->reg_f |= (mask)
#define CPU_MASK_RESET(obj, mask)       (obj)->reg_f &= ~(mask)
#define CPU_MASK_TOGGLE(obj, mask)      (obj)->reg_f ^= (mask)
#define CPU_MASK_SETON(obj, mask, val)  ((obj)->reg_f ^= (-(uint8_t)(EVAL_BOOL(val)) ^ (obj)->reg_f) & (mask))
#define CPU_MASK_GET(obj, mask)         ((obj)->reg_f & (mask))
*/
#define CPU_FLAG_SET(obj, bits)         BITS_SET((obj)->reg_f, bits)
#define CPU_FLAG_RESET(obj, bits)       BITS_RESET((obj)->reg_f, bits)
#define CPU_FLAG_FLIP(obj, bits)        BITS_FLIP((obj)->reg_f, bits)
#define CPU_FLAG_TEST(obj, bits)        BITS_TEST((obj)->reg_f, bits)

//Bitset Macros
/*
#define CPU_BIT_SET(obj, bit)           CPU_MASK_SET(obj, 1 << (bit))
#define CPU_BIT_RESET(obj, bit)         CPU_MASK_RESET(obj, 1 << (bit))
#define CPU_BIT_TOGGLE(obj, bit)        CPU_MASK_TOGGLE(obj, 1 << (bit))
#define CPU_BIT_SETON(obj, bit, val)    CPU_MASK_SETON(obj, 1 << (bit), val)
#define CPU_BIT_GET(obj, bit)           EVAL_BOOL(CPU_MASK_GET(obj, 1 << (bit)))
*/
#define CPU_BIT_SET(obj, bit)           BITS_SET_BIT((obj)->reg_f, bit)
#define CPU_BIT_RESET(obj, bit)         BITS_RESET_BIT((obj)->reg_f, bit)
#define CPU_BIT_FLIP(obj, bit)          BITS_FLIP_BIT((obj)->reg_f, bit)
#define CPU_BIT_TEST(obj, bit)          BITS_TEST_BIT((obj)->reg_f, bit)
#define CPU_BIT_COND(obj, bit, val)     BITS_COND_BIT((obj)->reg_f, bit, val)


#define CPU_FLAG_UPDATE(obj, s, z, a, p, c) \
(obj)->reg_f =\
((EVAL_BOOL(s) << CPU_SIGN_POS) |   \
(EVAL_BOOL(z) << CPU_ZERO_POS) |    \
(EVAL_BOOL(a) << CPU_AC_POS)   |    \
(EVAL_BOOL(p) << CPU_PARITY_POS) |  \
(EVAL_BOOL(c) << CPU_CARRY_POS)) | \
CPU_FLAG_DEFAULT


#define CPU_CORE_SIZE   offsetof(cpu_t, memory)
#define CPU_MEMSIZE     0x10000

typedef struct {
    /* Registers */
    uint8_t reg_a;
    uint8_t reg_f;
    uint8_t reg_b;
    uint8_t reg_c;
    uint8_t reg_d;
    uint8_t reg_e;
    uint8_t reg_h;
    uint8_t reg_l;
    uint16_t reg_pc;
    uint16_t reg_sp;

    /* Implementation flags */
    uint8_t halt;
    uint8_t inter;

    /* Memory Map */
    mmap_t memory;
    mmap_t io;
} cpu_t;

enum {
    OP_A,
    OP_B,
    OP_C,
    OP_D,
    OP_E,
    OP_H,
    OP_L,
    OP_M,
    OP_IMMEDIATE,
};

enum {
    OPX_BC,
    OPX_DE,
    OPX_HL,
    OPX_PSW,
    OPX_SP,
    OPX_IMMEDIATE,
};

enum {
    COND_Z,
    COND_NZ,
    COND_C,
    COND_NC,
    COND_PO,
    COND_PE,
    COND_P,
    COND_M,
};

#endif //VM_CPU_H