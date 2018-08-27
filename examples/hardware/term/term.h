//
// Created by Steven McLeod on 2018-03-02.
//

#ifndef VM8080_TERM_H
#define VM8080_TERM_H

#include <stdio.h>
#include <stdint.h>

#define TERM_BUF_SIZE 2

#define TERM_MEMSPACE(obj, origin) {&term_rd, &term_wr, &(obj), &(obj), (obj).regs, (origin), TERM_BUF_SIZE, MMAP_RDWR, "TERM"}

struct term_t {
    int inbuf;
    uint8_t regs[TERM_BUF_SIZE];

    FILE *logfile;
    int logdir;
    int logcount;
    char logbuf[16];
};

int term_init(struct term_t *obj, const char *logname);
int term_close(struct term_t *obj);
int term_rawmode(struct termios *term);
int term_restore(struct termios *term);

void term_rd(uint16_t pos, void *args);
void term_wr(uint16_t pos, void *args);

#endif //VM8080_TERM_H
