//
// Created by Steven McLeod on 2018-03-02.
//

#ifndef VM8080_TERM_H
#define VM8080_TERM_H

#include <stdint.h>

#define TERM_BUF_SIZE 2

#define TERM_MEMSPACE(obj, origin) {&term_rd, &term_wr, &(obj), &(obj), (obj).regs, (origin), TERM_BUF_SIZE, MMAP_RDWR}

struct term_t {
    int inbuf;
    uint8_t regs[TERM_BUF_SIZE];
};

int term_init(struct term_t *obj);

void term_rd(uint16_t pos, void *args);
void term_wr(uint16_t pos, void *args);

#endif //VM8080_TERM_H
