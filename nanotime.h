//
// Created by Steven McLeod on 2018-01-11.
//

#ifndef VM8080_NANOTIME_H
#define VM8080_NANOTIME_H

#define MS_TO_NS(x) ((x) * 1000000L)
#define US_TO_NS(x) ((x) * 1000L)

typedef struct {
    int nanosec;
    int sec;
} nanotime_t;

nanotime_t nanotime_add(nanotime_t a, nanotime_t b);
nanotime_t nanotime_sub(nanotime_t a, nanotime_t b);
nanotime_t nanotime_mulint(nanotime_t a, int b);
nanotime_t nanotime_divint(nanotime_t a, int b);
int nanotime_cmp(nanotime_t a, nanotime_t b);
nanotime_t nanotime_max(nanotime_t a, nanotime_t b);
nanotime_t nanotime_min(nanotime_t a, nanotime_t b);

#endif //VM8080_NANOTIME_H
