//
// Created by Steven on 25-Nov-17.
//

#include "mul.h"

//MEMFUNC(mul_rd, pos, params) {
void mul_rd(uint16_t pos, void *params) {
    hw_mul_t *obj = params;

    if(obj->changeflag) {
        obj->prod = (uint16_t) obj->regs[0] * (uint16_t) obj->regs[1];
        obj->changeflag = 0;
    }

    if(pos == 2) {
        obj->regs[2] = (uint8_t) (obj->prod & 0x00FF);
    } else if(pos == 3) {
        obj->regs[3] = (uint8_t) ((obj->prod & 0xFF00) >> 8);
    }
}

//MEMFUNC(mul_rd, pos, params) {
void mul_wr(uint16_t pos, void *params) {
    hw_mul_t *obj = params;

    if(pos == 0 || pos == 1) {
        obj->changeflag = 1;
    }
}