//
// Created by Steven McLeod on 2018-03-02.
//

#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include "term.h"

#define DIR_OUTPUT 0
#define DIR_INPUT 1
#define LOG_NEWCOUNT 16

#define CHARMODE 0

static int term_checkchar(int last);

static void term_logaction(struct term_t *obj, char c, int dir) {
    if(!obj->logfile) return;

    if(obj->logdir == -1 || obj->logdir != dir || obj->logcount >= LOG_NEWCOUNT) {
        if(obj->logcount) {
            for(int i = 0; i < LOG_NEWCOUNT - obj->logcount; ++i) {
                fputs("   ", obj->logfile);
            }

            for(int i = 0; i < obj->logcount; ++i) {
                fputc(isprint(obj->logbuf[i]) ? obj->logbuf[i] : '.', obj->logfile);
            }
        }
        fprintf(obj->logfile, "\n%s: ", dir == DIR_OUTPUT ? "OUT" : "IN ");
        obj->logdir = dir;
        obj->logcount = 0;
    }

    fprintf(obj->logfile, "%02x ", c);
    fflush(obj->logfile);
    obj->logbuf[obj->logcount] = c;
    ++obj->logcount;
}

int term_rawmode(struct termios *term) {
    struct termios term_new;

    if(!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: Not a tty\n");
        return -1;
    }

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    if(tcgetattr(STDIN_FILENO, term)) {
        fprintf(stderr, "Error: Could not get term attr\n");
        return -1;
    }

    term_new = *term;

    term_new.c_iflag |= (INLCR);
    term_new.c_oflag &= ~(OPOST);
    term_new.c_lflag &= ~(ICANON | ECHO);
    term_new.c_cc[VTIME] = 0;
    term_new.c_cc[VMIN] = 1;

    if(tcsetattr(STDIN_FILENO, TCSANOW, &term_new)) {
        fprintf(stderr, "Error: Could not set term attr\n");
        return -1;
    }

    return 0;
}

int term_restore(struct termios *term) {
    return tcsetattr(STDIN_FILENO, TCSANOW, term);
}

#if CHARMODE == 0
/* getchar method */
static int term_charavail(struct term_t *obj) {
    int c = getchar();
    if(c != -1) {
        if(c == '\n') c = '\r';
        obj->inbuf = c;
    }

    return c != -1;
}

#elif CHARMODE == 1
/* poll method */
static int term_charavail(struct term_t *obj) {
    struct pollfd inpoll = {STDIN_FILENO, POLLIN, 0};
    if(!poll(&inpoll, 1, 0)) {
        if(!inpoll.revents)
            return 0;
    }

    char c;
    ssize_t br = read(STDIN_FILENO, &c, 1);
    if(br >= 0) {
        if(c == '\n') c = '\r';
        obj->inbuf = c;
    }

    return br >= 0;
}

#elif CHARMODE == 2
/* select method */
static int term_charavail(struct term_t *obj) {
    fd_set inset;
    struct timeval intime = {0, 0};
    FD_ZERO(&inset);
    FD_SET(STDIN_FILENO, &inset);

    if(!select(1, &inset, NULL, NULL, &intime)) {
        return 0;
    }

    char c;
    ssize_t br = read(STDIN_FILENO, &c, 1);
    if(br >= 0) {
        if(c == '\n') c = '\r';
        obj->inbuf = c;
    }

    return br >= 0;
}

#else
#error "INVALID MODE"
#endif

int term_init(struct term_t *obj, const char *logname) {
    memset(obj, 0, sizeof(struct term_t));
    obj->inbuf = -1;

    // Make sure not null string
    if(logname && logname[0]) {
        obj->logfile = fopen(logname, "w");
        if(!obj->logfile) return -1;
        obj->logdir = -1;
        obj->logcount = 0;
        fputs("UART Logfile", obj->logfile);
    }
    return 0;
}

int term_close(struct term_t *obj) {
    if(obj->logcount) {
        for(int i = 0; i < LOG_NEWCOUNT - obj->logcount; ++i) {
            fputs("   ", obj->logfile);
        }

        for(int i = 0; i < obj->logcount; ++i) {
            fputc(isprint(obj->logbuf[i]) ? obj->logbuf[i] : '.', obj->logfile);
        }

        fputc('\n', obj->logfile);
    }

    fclose(obj->logfile);
    obj->logfile = NULL;
    return 0;
}

void term_rd(uint16_t pos, void *args) {
    struct term_t *obj = args;
    uint8_t *loc = &obj->regs[pos];

    /*
     * Reg 0: Input register
     * Reg 1: Status register
     *
     */

    if(pos == 0) {
        if(obj->inbuf != -1 || term_charavail(obj)) {
            // Is there already a buffered character OR
            // Is there a char available
            *loc = (uint8_t) obj->inbuf;
            obj->inbuf = -1;
        }

        term_logaction(obj, *loc, DIR_INPUT);
    } else if(pos == 1) {
        // Is there already a buffered character OR
        // Is there a char available
        if(obj->inbuf != -1 || term_charavail(obj)) {
            *loc = 1;
        } else {
            *loc = 0;
        }
    }
}

void term_wr(uint16_t pos, void *args) {
    struct term_t *obj = args;
    uint8_t val = obj->regs[pos];

    if(pos == 0) {
        // Don't print cret
        //if(val != '\r')
#if CHARMODE == 0
        putchar(val);
#else
        write(STDIN_FILENO, &val, 1);
#endif

        term_logaction(obj, val, DIR_OUTPUT);
    }
}