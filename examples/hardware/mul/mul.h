//
// Created by Steven on 25-Nov-17.
//

#ifndef VM2_MUL_H
#define VM2_MUL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool changeflag;
    uint16_t prod;
    uint8_t regs[4];
} hw_mul_t;

void mul_rd(uint16_t pos, void *params);
void mul_wr(uint16_t pos, void *params);

//MEMFUNC(mul_rd, pos, params);
//MEMFUNC(mul_wr, pos, params);

#endif //VM2_MUL_H
