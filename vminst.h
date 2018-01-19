#ifndef VM8080_VMINSTR_H
#define VM8080_VMINSTR_H

#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"

#define vm_terminate(code, msg, obj) \
do {\
    fprintf(stderr, "Error %d: %s\nFile: %s, Line: %d, Func: %s\nPC: 0x%04hx", code, msg, __FILE__, __LINE__, __func__, (obj)->reg_pc);\
    exit(EXIT_FAILURE);\
} while(0)

/*
 *  8-bit Register Operations
 *
 */

void vm_mov(cpu_t *obj, int destmode, int srcmode);
void vm_add(cpu_t *obj, int mode);
void vm_adc(cpu_t *obj, int mode);
void vm_sub(cpu_t *obj, int mode);
void vm_sbb(cpu_t *obj, int mode);
void vm_inr(cpu_t *obj, int mode);
void vm_dcr(cpu_t *obj, int mode);
void vm_ana(cpu_t *obj, int mode);
void vm_ora(cpu_t *obj, int mode);
void vm_xra(cpu_t *obj, int mode);
void vm_cmp(cpu_t *obj, int mode);

void vm_rlc(cpu_t *obj);
void vm_rrc(cpu_t *obj);
void vm_ral(cpu_t *obj);
void vm_rar(cpu_t *obj);

void vm_lda(cpu_t *obj);
void vm_sta(cpu_t *obj);
void vm_ldax(cpu_t *obj, int mode);
void vm_stax(cpu_t *obj, int mode);

/*
 *  16-bit Register Operations
 *
 */

void vm_lxi(cpu_t *obj, int mode);
void vm_lhld(cpu_t *obj);
void vm_shld(cpu_t *obj);
void vm_sphl(cpu_t *obj);
void vm_dad(cpu_t *obj, int mode);
void vm_inx(cpu_t *obj, int mode);
void vm_dcx(cpu_t *obj, int mode);

void vm_xchg(cpu_t *obj);
void vm_xthl(cpu_t *obj);


/*
 *  Branch Operations
 *
 */

void vm_jmp(cpu_t *obj);
void vm_jcond(cpu_t *obj, int cond);
void vm_pchl(cpu_t *obj);
void vm_call(cpu_t *obj);
void vm_ccond(cpu_t *obj, int cond);
void vm_ret(cpu_t *obj);
void vm_rcond(cpu_t *obj, int cond);

void vm_rst(cpu_t *obj, int rstno);

/*
 *  Misc. Operations
 *
 */

void vm_push(cpu_t *obj, int mode);
void vm_pop(cpu_t *obj, int mode);

void vm_in(cpu_t *obj);
void vm_out(cpu_t *obj);

void vm_di(cpu_t *obj);
void vm_ei(cpu_t *obj);
void vm_nop(cpu_t *obj);
void vm_hlt(cpu_t *obj);
void vm_daa(cpu_t *obj);
void vm_cma(cpu_t *obj);
void vm_stc(cpu_t *obj);
void vm_cmc(cpu_t *obj);

#endif // VM8080_VMINSTR_H