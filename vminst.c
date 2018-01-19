#include "vminst.h"
#include "bitset.h"
#include <stdbool.h>

#define NIBBLE_LO(n) ((n) & 0x0F)
#define NIBBLE_HI(n) ((n) & 0xF0)

//TODO Incorporate AC flag
#define VM_UPDATE_ALL(obj, val)         CPU_FLAG_UPDATE(obj, (val) & 0x80, !((val) & 0xff), 0, (val) & 0x01, (val) & 0x100)
#define VM_UPDATE_NO_CARRY(obj, val)    CPU_FLAG_UPDATE(obj, (val) & 0x80, !((val) & 0xff), 0, (val) & 0x01, CPU_BIT_TEST(obj, CPU_CARRY_POS))

/*
 *  Static Functions
 *
 */

//Memory Access
static uint8_t  vm_getmem8(cpu_t *obj, uint16_t addr);
static uint16_t vm_getmem16(cpu_t *obj, uint16_t addr);
static void     vm_putmem8(cpu_t *obj, uint16_t addr, uint8_t data);
static void     vm_putmem16(cpu_t *obj, uint16_t addr, uint16_t data);
static uint8_t  vm_getop8(cpu_t *obj);
static uint16_t vm_getop16(cpu_t *obj);

//Parameter Access
static uint16_t vm_getregpair(cpu_t *obj, int mode);
static void     vm_putregpair(cpu_t *obj, int mode, uint16_t data);
static void     vm_pushpc(cpu_t *obj);
static void     vm_poppc(cpu_t *obj);
static uint8_t  vm_getval8(cpu_t *obj, int mode);
static void     vm_putval8(cpu_t *obj, int mode, uint8_t data);
static uint16_t vm_getval16(cpu_t *obj, int mode);
static void     vm_putval16(cpu_t *obj, int mode, uint16_t data);
static bool     vm_getcond(cpu_t *obj, int cond);

/*
 *  8-bit Register Operations
 *
 */

void vm_mov(cpu_t *obj, int destmode, int srcmode) {
    uint8_t srcval = vm_getval8(obj, srcmode);
    vm_putval8(obj, destmode, srcval);
}

void vm_add(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a + srcval;

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_adc(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a + srcval + CPU_BIT_TEST(obj, CPU_CARRY_POS);

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_sub(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a - srcval;

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_sbb(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a - srcval - CPU_BIT_TEST(obj, CPU_CARRY_POS);

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_inr(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = srcval + 1u;

    vm_putval8(obj, mode, (uint8_t) res);

    //Set Flags
    VM_UPDATE_NO_CARRY(obj, res);
}

void vm_dcr(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = srcval - 1u;

    vm_putval8(obj, mode, (uint8_t) res);

    //Set Flags
    VM_UPDATE_NO_CARRY(obj, res);
}

void vm_ana(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a & srcval;

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_ora(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a | srcval;

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_xra(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a ^ srcval;

    obj->reg_a = (uint8_t) res;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

void vm_cmp(cpu_t *obj, int mode) {
    uint8_t srcval = vm_getval8(obj, mode);
    unsigned res = obj->reg_a - srcval;

    //Set Flags
    VM_UPDATE_ALL(obj, res);
}

//TODO Rotates Broken
void vm_rlc(cpu_t *obj) {
    unsigned rotatebit = BITS_TEST_BIT(obj->reg_a, 7);
    unsigned carrybit = rotatebit;

    //Bit0 = Bit7, CF = Bit7
    unsigned res = (obj->reg_a << 1) | rotatebit;
    CPU_BIT_COND(obj, CPU_CARRY_POS, carrybit);
    obj->reg_a = (uint8_t) res;
}

void vm_rrc(cpu_t *obj) {
    unsigned rotatebit = BITS_TEST_BIT(obj->reg_a, 0);
    unsigned carrybit = rotatebit;

    //Bit7 = Bit0, CF = Bit0
    unsigned res = (obj->reg_a >> 1) | (rotatebit << 7);
    CPU_BIT_COND(obj, CPU_CARRY_POS, obj->reg_a & 0x01u);
    obj->reg_a = (uint8_t) res;
}

void vm_ral(cpu_t *obj) {
    unsigned rotatebit = CPU_BIT_TEST(obj, CPU_CARRY_POS);
    unsigned carrybit = BITS_TEST_BIT(obj->reg_a, 7);

    //Bit0 = CF, CF = Bit7
    unsigned res = (obj->reg_a << 1) | rotatebit;
    CPU_BIT_COND(obj, CPU_CARRY_POS, carrybit);
    obj->reg_a = (uint8_t) res;
}

void vm_rar(cpu_t *obj) {
    unsigned rotatebit = CPU_BIT_TEST(obj, CPU_CARRY_POS);
    unsigned carrybit = BITS_TEST_BIT(obj->reg_a, 0);

    //Bit7 = CF, CF = Bit0
    unsigned res = (obj->reg_a >> 1) | (rotatebit << 7);
    CPU_BIT_COND(obj, CPU_CARRY_POS, carrybit);
    obj->reg_a = (uint8_t) res;
}

void vm_lda(cpu_t *obj) {
    uint16_t addr = vm_getop16(obj);
    uint8_t srcval = vm_getmem8(obj, addr);
    obj->reg_a = srcval;
}

void vm_sta(cpu_t *obj) {
    uint16_t addr = vm_getop16(obj);
    uint8_t srcval = obj->reg_a;
    vm_putmem8(obj, addr, srcval);
}

void vm_ldax(cpu_t *obj, int mode) {
    uint16_t addr = vm_getregpair(obj, mode);
    uint8_t srcval = vm_getmem8(obj, addr);
    obj->reg_a = srcval;
}

void vm_stax(cpu_t *obj, int mode) {
    uint16_t addr = vm_getregpair(obj, mode);
    uint8_t srcval = obj->reg_a;
    vm_putmem8(obj, addr, srcval);
}


/*
 *  16-bit Register Operations
 *
 */

void vm_lxi(cpu_t *obj, int mode) {
    uint16_t srcval = vm_getop16(obj);
    vm_putval16(obj, mode, srcval);
}

void vm_lhld(cpu_t *obj) {
    uint16_t addr = vm_getop16(obj);
    uint16_t srcval = vm_getmem16(obj, addr);
    vm_putregpair(obj, OPX_HL, srcval);
}

void vm_shld(cpu_t *obj) {
    uint16_t addr = vm_getop16(obj);
    uint16_t srcval = vm_getregpair(obj, OPX_HL);
    vm_putmem16(obj, addr, srcval);
}

void vm_sphl(cpu_t *obj) {
    uint16_t srcval = vm_getregpair(obj, OPX_HL);
    obj->reg_sp = srcval;
}

void vm_dad(cpu_t *obj, int mode) {
    uint16_t srcval = vm_getval16(obj, mode);
    unsigned res = vm_getregpair(obj, OPX_HL) + srcval;
    vm_putregpair(obj, OPX_HL, (uint16_t) res);

    //Set Flags
    CPU_BIT_COND(obj, CPU_CARRY_POS, res & 0x10000);
}

void vm_inx(cpu_t *obj, int mode) {
    uint16_t srcval = vm_getval16(obj, mode);
    unsigned res = srcval + 1u;
    vm_putregpair(obj, mode, (uint16_t) res);
}

void vm_dcx(cpu_t *obj, int mode) {
    uint16_t srcval = vm_getval16(obj, mode);
    unsigned res = srcval - 1u;
    vm_putregpair(obj, mode, (uint16_t) res);
}


/*
 *  Branch Operations
 *
 */

void vm_jmp(cpu_t *obj) {
    obj->reg_pc = vm_getop16(obj);
}

void vm_jcond(cpu_t *obj, int cond) {
    uint16_t addr = vm_getop16(obj);
    if(vm_getcond(obj, cond)) {
        obj->reg_pc = addr;
    }
}

void vm_pchl(cpu_t *obj) {
    uint16_t addr = vm_getregpair(obj, OPX_HL);
    obj->reg_pc = addr;
}

void vm_call(cpu_t *obj) {
    uint16_t addr = vm_getop16(obj);
    vm_pushpc(obj);
    obj->reg_pc = addr;
}

void vm_ccond(cpu_t *obj, int cond) {
    uint16_t addr = vm_getop16(obj);
    if(vm_getcond(obj, cond)) {
        vm_pushpc(obj);
        obj->reg_pc = addr;
    }
}

void vm_ret(cpu_t *obj) {
    vm_poppc(obj);
}

void vm_rcond(cpu_t *obj, int cond) {
    if(vm_getcond(obj, cond)) {
        vm_poppc(obj);
    }
}

void vm_rst(cpu_t *obj, int rstno) {
    switch(rstno) {
        case 0: obj->reg_pc = 0x00; break;
        case 1: obj->reg_pc = 0x08; break;
        case 2: obj->reg_pc = 0x10; break;
        case 3: obj->reg_pc = 0x18; break;
        case 4: obj->reg_pc = 0x20; break;
        case 5: obj->reg_pc = 0x28; break;
        case 6: obj->reg_pc = 0x30; break;
        case 7: obj->reg_pc = 0x38; break;
        default: vm_terminate(4, "Invalid RST.", obj);
    }
}

/*
 *  Misc. Operations
 *
 */

void vm_push(cpu_t *obj, int mode) {
    uint16_t srcval = vm_getregpair(obj, mode);
    obj->reg_sp -= 2;
    vm_putmem16(obj, obj->reg_sp, srcval);
}

void vm_pop(cpu_t *obj, int mode) {
    uint16_t srcval = vm_getmem16(obj, obj->reg_sp);
    obj->reg_sp += 2;
    vm_putregpair(obj, mode, srcval);
}

void vm_in(cpu_t *obj) {
    uint8_t port = vm_getop8(obj);
    int c = getchar();
    if(c == -1) {
        vm_terminate(3, "Unexpected EOF", obj);
    }
    obj->reg_a = (uint8_t) c;
}

void vm_out(cpu_t *obj) {
    uint8_t port = vm_getop8(obj);
    putchar(obj->reg_a);
}

void vm_xchg(cpu_t *obj) {
    uint16_t de = vm_getregpair(obj, OPX_DE);
    uint16_t hl = vm_getregpair(obj, OPX_HL);
    vm_putregpair(obj, OPX_DE, hl);
    vm_putregpair(obj, OPX_HL, de);
}

void vm_xthl(cpu_t *obj) {
    uint16_t hl = vm_getregpair(obj, OPX_HL);
    uint16_t st = vm_getmem16(obj, obj->reg_sp);
    vm_putregpair(obj, OPX_HL, st);
    vm_putmem16(obj, obj->reg_sp, st);
}

void vm_di(cpu_t *obj) {
    obj->inter = 0;
}

void vm_ei(cpu_t *obj) {
    obj->inter = 1;
}

void vm_nop(cpu_t *obj) {
    return;
}

void vm_hlt(cpu_t *obj) {
    obj->halt = 1;
}

//Implementation here: https://stackoverflow.com/questions/8119577/z80-daa-instruction
void vm_daa(cpu_t *obj) {
    if(NIBBLE_LO(obj->reg_a) > 0x09 || CPU_BIT_TEST(obj, CPU_AC_POS)) {
        obj->reg_a += 0x06;
    }

    if(NIBBLE_HI(obj->reg_a) > 0x90 || CPU_BIT_TEST(obj, CPU_CARRY_POS)) {
        obj->reg_a += 0x60;
    }
}

void vm_cma(cpu_t *obj) {
    uint8_t srcval = obj->reg_a;
    obj->reg_a = ~srcval;
}

void vm_stc(cpu_t *obj) {
    CPU_BIT_SET(obj, CPU_CARRY_POS);
}

void vm_cmc(cpu_t *obj) {
    CPU_BIT_FLIP(obj, CPU_CARRY_POS);
}

/*
 *  Static Function Implementation
 *
 */

//Memory Access
uint8_t vm_getmem8(cpu_t *obj, uint16_t addr) {
    uint8_t b = mmap_read(&obj->memory, addr);
    return b;
}

uint16_t vm_getmem16(cpu_t *obj, uint16_t addr) {
    uint8_t lb = mmap_read(&obj->memory, addr);
    uint8_t hb = mmap_read(&obj->memory, addr + 1u);
    uint16_t w = (uint16_t) (lb) | (uint16_t) (hb << 8);
    return w;
}

void vm_putmem8(cpu_t *obj, uint16_t addr, uint8_t data) {
    mmap_write(&obj->memory, addr, data);
}

void vm_putmem16(cpu_t *obj, uint16_t addr, uint16_t data) {
    uint8_t lb = data & 0x00FF;
    uint8_t hb = (data & 0xFF00) >> 8;
    mmap_write(&obj->memory, addr, lb);
    mmap_write(&obj->memory, addr + 1u, hb);
}

uint8_t vm_getop8(cpu_t *obj) {
    uint8_t b = mmap_read(&obj->memory, obj->reg_pc++);
    return b;
}

uint16_t vm_getop16(cpu_t *obj) {
    uint8_t lb = mmap_read(&obj->memory, obj->reg_pc++);
    uint8_t hb = mmap_read(&obj->memory, obj->reg_pc++);
    uint16_t w = (uint16_t) (lb) | (uint16_t) (hb << 8);
    return w;
}

//Parameter Access
uint16_t vm_getregpair(cpu_t *obj, int mode) {
    uint8_t lb, hb;

    switch(mode) {
        case OPX_BC:
            lb = obj->reg_c;
            hb = obj->reg_b;
            break;

        case OPX_DE:
            lb = obj->reg_e;
            hb = obj->reg_d;
            break;

        case OPX_HL:
            lb = obj->reg_l;
            hb = obj->reg_h;
            break;

        case OPX_PSW:
            lb = obj->reg_f;
            hb = obj->reg_a;
            break;

        default:
            vm_terminate(2, "Invalid Addressing Mode", obj);
    }

    return (uint16_t) (lb) | (uint16_t) (hb << 8);
}

void vm_putregpair(cpu_t *obj, int mode, uint16_t data) {
    uint8_t lb = data & 0x00FF;
    uint8_t hb = (data & 0xFF00) >> 8;

    switch(mode) {
        case OPX_BC:
            obj->reg_c = lb;
            obj->reg_b = hb;
            break;

        case OPX_DE:
            obj->reg_e = lb;
            obj->reg_d = hb;
            break;

        case OPX_HL:
            obj->reg_l = lb;
            obj->reg_h = hb;
            break;

        case OPX_PSW:
            obj->reg_f = lb;
            obj->reg_a = hb;
            break;

        default:
            vm_terminate(2, "Invalid Addressing Mode", obj);
    }
}

void vm_pushpc(cpu_t *obj) {
    obj->reg_sp -= 2;
    vm_putmem16(obj, obj->reg_sp, obj->reg_pc);
}

void vm_poppc(cpu_t *obj) {
    obj->reg_pc = vm_getmem16(obj, obj->reg_sp);
    obj->reg_sp += 2;
}

uint8_t vm_getval8(cpu_t *obj, int mode) {
    switch(mode) {
        case OP_A:          return obj->reg_a;
        case OP_B:          return obj->reg_b;
        case OP_C:          return obj->reg_c;
        case OP_D:          return obj->reg_d;
        case OP_E:          return obj->reg_e;
        case OP_H:          return obj->reg_h;
        case OP_L:          return obj->reg_l;
        case OP_M:          return vm_getmem8(obj, vm_getregpair(obj, OPX_HL));
        case OP_IMMEDIATE:  return vm_getop8(obj);
        default:            vm_terminate(2, "Invalid Addressing Mode", obj);
    }
}

void vm_putval8(cpu_t *obj, int mode, uint8_t data) {
    switch(mode) {
        case OP_A:          obj->reg_a = data; break;
        case OP_B:          obj->reg_b = data; break;
        case OP_C:          obj->reg_c = data; break;
        case OP_D:          obj->reg_d = data; break;
        case OP_E:          obj->reg_e = data; break;
        case OP_H:          obj->reg_h = data; break;
        case OP_L:          obj->reg_l = data; break;
        case OP_M:          mmap_write(&obj->memory, vm_getregpair(obj, OPX_HL), data); break;
        default:            vm_terminate(2, "Invalid Addressing Mode", obj);
    }
}

uint16_t vm_getval16(cpu_t *obj, int mode) {
    switch(mode) {
        case OPX_BC:
        case OPX_DE:
        case OPX_HL:
        case OPX_PSW:
                            return vm_getregpair(obj, mode);

        case OPX_SP:        return obj->reg_sp;
        case OPX_IMMEDIATE: return vm_getop16(obj);
        default:            vm_terminate(2, "Invalid Addressing Mode", obj);
    }
}

void vm_putval16(cpu_t *obj, int mode, uint16_t data) {
    switch(mode) {
        case OPX_BC:
        case OPX_DE:
        case OPX_HL:
        case OPX_PSW:
                            vm_putregpair(obj, mode, data); break;

        case OPX_SP:        obj->reg_sp = data; break;
        default:            vm_terminate(2, "Invalid Addressing Mode", obj);
    }
}

bool vm_getcond(cpu_t *obj, int cond) {
    switch(cond) {
        case COND_NZ:   return !CPU_BIT_TEST(obj, CPU_ZERO_POS);
        case COND_Z:    return CPU_BIT_TEST(obj, CPU_ZERO_POS);
        case COND_NC:   return !CPU_BIT_TEST(obj, CPU_CARRY_POS);
        case COND_C:    return CPU_BIT_TEST(obj, CPU_CARRY_POS);
        case COND_PO:   return !CPU_BIT_TEST(obj, CPU_PARITY_POS);
        case COND_PE:   return CPU_BIT_TEST(obj, CPU_PARITY_POS);
        case COND_P:    return !CPU_BIT_TEST(obj, CPU_SIGN_POS);
        case COND_M:    return CPU_BIT_TEST(obj, CPU_SIGN_POS);
        default:        vm_terminate(5, "Invalid Condition Mode", obj);
    }
}