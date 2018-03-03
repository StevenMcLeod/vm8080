//
// Created by Steven McLeod on 2018-03-02.
//

#include <stdio.h>
#include <string.h>
#include "term.h"

static int term_checkchar(int last);

int term_init(struct term_t *obj) {
    memset(obj, 0, sizeof(struct term_t));
    obj->inbuf = -1;
    return 0;
}

void term_rd(uint16_t pos, void *args) {
    struct term_t *obj = args;
    uint8_t *loc = &obj->regs[pos];

    if(pos == 0) {
        // Is there already a buffered character
        if(obj->inbuf != -1) {
            *loc = (uint8_t) obj->inbuf;
            obj->inbuf = -1;
        } else {
            int c = getchar();

            // Is there a character?
            if(c != -1) {
                *loc = (uint8_t) c;
            }
        }
    } else if(pos == 1) {
        // Is there already a buffered character
        if(obj->inbuf != -1) {
            *loc = 1;
        } else {
            int c = getchar();

            // Is there a character?
            if(c != -1) {
                *loc = 1;
                obj->inbuf = c;
            } else {
                *loc = 0;
            }
        }
    }

    if(pos == 0 && *loc == '\n') {
        *loc = '\r';
    }
}

void term_wr(uint16_t pos, void *args) {
    struct term_t *obj = args;
    uint8_t val = obj->regs[pos];

    if(pos == 0) {
        if(val != '\r')
            putchar(val);
    }
}