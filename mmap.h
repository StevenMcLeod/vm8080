//
// Created by Steven McLeod on 2017-11-21.
//

#ifndef VM8080_MMAP_H
#define VM8080_MMAP_H

#include <stdint.h>

#define MMAP_READ       0x01
#define MMAP_WRITE      0x02
#define MMAP_RDWR       0x03
#define MMAP_ALLOC      0x40
#define MMAP_FREE       0x80
#define MMAP_ALLOCFREE  0xC0

#define MEMFUNC(n, p1, p2) void n(uint16_t p1, void *p2)
typedef void (*memfunc_t)(uint16_t, void *);

typedef struct {
    memfunc_t rd_func, wr_func;
    void *rd_param, *wr_param;
    uint8_t *memory;
    uint16_t origin, size;
    uint8_t flags;
    const char *name;
} memspace_t;

typedef struct _memnode {
    struct _memnode *next;
    memspace_t elem;
} _memnode_t;

typedef struct {
    _memnode_t *list;
    unsigned size;
} mmap_t;

int mmap_init(mmap_t *obj);
int mmap_destroy(mmap_t *obj);

//int mmap_add(mmap_t *obj, uint16_t origin, uint16_t size, uint8_t perms, uint8_t *buf);
int mmap_add(mmap_t *obj, const memspace_t *elem);
int mmap_remove(mmap_t *obj, uint16_t origin, uint16_t size);
int mmap_valid(mmap_t *obj, uint16_t addr, uint8_t flags);
void mmap_print(mmap_t *obj);

uint8_t mmap_read(mmap_t *obj, uint16_t addr);
void mmap_write(mmap_t *obj, uint16_t addr, uint8_t data);
uint8_t mmap_read_noexec(mmap_t *obj, uint16_t addr);
void mmap_write_noexec(mmap_t *obj, uint16_t addr, uint8_t data);

void mmap_read_block(mmap_t *obj, uint16_t addr, uint16_t len, uint8_t *buf);
void mmap_write_block(mmap_t *obj, uint16_t addr, uint16_t len, const uint8_t *buf);
void mmap_read_block_noexec(mmap_t *obj, uint16_t addr, uint16_t len, uint8_t *buf);
void mmap_write_block_noexec(mmap_t *obj, uint16_t addr, uint16_t len, const uint8_t *buf);

uint8_t *mmap_debug_read_block(mmap_t *obj, uint16_t addr, uint16_t len);

#endif //VM8080_MMAP_H
