//
// Created by Steven McLeod on 2017-11-21.
//

#include "mmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MMAP_EMPTY_BUS 0xFF

static struct _memnode *mmap_create_memnode(_memnode_t *prev, _memnode_t *next, const memspace_t *elem);

int mmap_init(mmap_t *obj) {
    obj->list = NULL;
    obj->size = 0;
    return 0;
}

int mmap_destroy(mmap_t *obj) {
    struct _memnode *tofree = obj->list;
    while(tofree) {
        struct _memnode *temp = tofree->next;

        if(tofree->elem.flags & MMAP_FREE) {
            free(tofree->elem.memory);
        }

        free(tofree);
        tofree = temp;
    }

    return 0;
}

int mmap_add(mmap_t *obj, const memspace_t *elem) {
    if(elem->size == 0) {
        return 0;
    }

    if(!obj->list) {
        if(elem->origin <= UINT16_MAX - elem->size + 1) {
/*
            _memnode_t *toinsert = malloc(sizeof(_memnode_t));
            if(!toinsert) {
                return -1;
            }

            obj->list = toinsert;
            toinsert->next = NULL;
            memcpy(&toinsert->elem, elem, sizeof(memspace_t));
            ++obj->size;
*/

            _memnode_t *newnode = mmap_create_memnode(NULL, NULL, elem);
            if(!newnode) {
                return -1;
            }

            obj->list = newnode;
            ++obj->size;

            return 0;
        } else {
            return -1;
        }
    } else {
        _memnode_t *left = NULL;
        _memnode_t *right = obj->list;

        //Case 1: Insert at front of list.
        if(elem->origin + elem->size - 1 < right->elem.origin) {
            //Check for overflow
            if(elem->origin <= UINT16_MAX - elem->size + 1) {
                //Insert at front of list.
                /*
                _memnode_t *toinsert = malloc(sizeof(_memnode_t));
                if(!toinsert) {
                    return -1;
                }

                obj->list = toinsert;
                toinsert->next = right;
                memcpy(&toinsert->elem, elem, sizeof(memspace_t));
                 */
                _memnode_t *newnode = mmap_create_memnode(NULL, right, elem);
                if(!newnode) {
                    return -1;
                }

                obj->list = newnode;
                ++obj->size;
                return 0;
            } else {
                return -1;
            }
        }

        //Case 2: Insert between two elements.
        left = right;
        right = right->next;
        while(right) {
            uint16_t index_left_bound = left->elem.origin + left->elem.size - 1;
            uint16_t index_right_bound = right->elem.origin;
            uint16_t elem_left_bound = elem->origin;
            uint16_t elem_right_bound = elem->origin + elem->size - 1;

            //Check if between
            if(index_left_bound <= elem_right_bound && index_right_bound >= elem_left_bound) {
                //Insert between
                /*
                _memnode_t *toinsert = malloc(sizeof(_memnode_t));
                if(!toinsert) {
                    return -1;
                }

                left->next = toinsert;
                toinsert->next = right;
                memcpy(&toinsert->elem, elem, sizeof(memspace_t));
                */
                _memnode_t *newnode = mmap_create_memnode(left, right, elem);
                if(!newnode) {
                    return -1;
                }

                ++obj->size;
                return 0;
            }

            left = right;
            right = right->next;
        }

        //Case 3: Insert at end of list.
        if(left->elem.origin + left->elem.size - 1 < elem->origin) {
            //Check for overflow
            if(elem->origin <= UINT16_MAX - elem->size + 1) {
                //Insert at end of list.
                /*
                _memnode_t *toinsert = malloc(sizeof(_memnode_t));
                if(!toinsert) {
                    return -1;
                }

                left->next = toinsert;
                toinsert->next = NULL;
                memcpy(&toinsert->elem, elem, sizeof(memspace_t));
                 */
                _memnode_t *newnode = mmap_create_memnode(left, NULL, elem);
                if(!newnode) {
                    return -1;
                }

                ++obj->size;
                return 0;
            } else {
                return -1;
            }
        }

        //Does not fit into memory map
        return -1;
    }
}

int mmap_remove(mmap_t *obj, uint16_t origin, uint16_t size) {

}

int mmap_valid(mmap_t *obj, uint16_t addr, uint8_t flags) {
    _memnode_t *index = obj->list;
    while(index) {
        uint16_t left_bound = index->elem.origin;
        uint16_t right_bound = index->elem.origin + index->elem.size - 1;
        if(left_bound <= addr && addr <= right_bound) {
            return obj->list->elem.flags & flags;
        }

        index = index->next;
    }

    return 0;
}

void mmap_print(mmap_t *obj) {
    printf("Memory Spaces: %u\n", obj->size);

    _memnode_t *index = obj->list;
    while(index) {
        char readc = (index->elem.flags & MMAP_READ) ? 'r' : ' ';
        char writec = (index->elem.flags & MMAP_WRITE) ? 'w' : ' ';
        char allocc = (index->elem.flags & MMAP_ALLOC) ? 'a' : ' ';
        char freec = (index->elem.flags & MMAP_FREE) ? 'f' : ' ';
        printf("Origin: 0x%04hx Size: 0x%04hx Flags: %c%c%c%c\n", index->elem.origin, index->elem.size, readc, writec, allocc, freec);
        index = index->next;
    }
}

uint8_t mmap_read(mmap_t *obj, uint16_t addr) {
    _memnode_t *index = obj->list;
    while(index) {
        uint16_t left_bound = index->elem.origin;
        uint16_t right_bound = index->elem.origin + index->elem.size - 1;
        if(left_bound <= addr && addr <= right_bound) {
            if(!(index->elem.flags & MMAP_READ)) {
                break;
            }

            uint16_t pos = addr - index->elem.origin;
            if(index->elem.rd_func) {
                (*index->elem.rd_func)(pos, index->elem.rd_param);
            }
            return index->elem.memory[pos];
        }

        index = index->next;
    }

    return MMAP_EMPTY_BUS;
}

void mmap_write(mmap_t *obj, uint16_t addr, uint8_t data) {
    _memnode_t *index = obj->list;
    while(index) {
        uint16_t left_bound = index->elem.origin;
        uint16_t right_bound = index->elem.origin + index->elem.size - 1;
        if(left_bound <= addr && addr <= right_bound) {
            if((index->elem.flags & MMAP_WRITE)) {
                uint16_t pos = addr - index->elem.origin;
                index->elem.memory[pos] = data;
                if(index->elem.wr_func) {
                    (*index->elem.wr_func)(pos, index->elem.wr_param);
                }
            }

            return;
        }

        index = index->next;
    }
}

uint8_t mmap_read_noexec(mmap_t *obj, uint16_t addr) {
    _memnode_t *index = obj->list;
    while(index) {
        uint16_t left_bound = index->elem.origin;
        uint16_t right_bound = index->elem.origin + index->elem.size - 1;
        if(left_bound <= addr && addr <= right_bound) {
            if(!(index->elem.flags & MMAP_READ)) {
                break;
            }

            uint16_t pos = addr - index->elem.origin;
            return index->elem.memory[pos];
        }

        index = index->next;
    }

    return MMAP_EMPTY_BUS;
}

void mmap_write_noexec(mmap_t *obj, uint16_t addr, uint8_t data) {
    _memnode_t *index = obj->list;
    while(index) {
        uint16_t left_bound = index->elem.origin;
        uint16_t right_bound = index->elem.origin + index->elem.size - 1;
        if(left_bound <= addr && addr <= right_bound) {
            if((index->elem.flags & MMAP_WRITE)) {
                uint16_t pos = addr - index->elem.origin;
                index->elem.memory[pos] = data;
            }

            return;
        }

        index = index->next;
    }
}

void mmap_read_block(mmap_t *obj, uint16_t addr, uint16_t len, uint8_t *buf) {
    for(uint16_t i = 0; i < len; ++i) {
        buf[i] = mmap_read(obj, addr + i);
    }
}

void mmap_write_block(mmap_t *obj, uint16_t addr, uint16_t len, const uint8_t *buf) {
    for(uint16_t i = 0; i < len; ++i) {
        mmap_write(obj, addr + i, buf[i]);
    }
}

void mmap_read_block_noexec(mmap_t *obj, uint16_t addr, uint16_t len, uint8_t *buf) {
    for(uint16_t i = 0; i < len; ++i) {
        buf[i] = mmap_read_noexec(obj, addr + i);
    }
}

void mmap_write_block_noexec(mmap_t *obj, uint16_t addr, uint16_t len, const uint8_t *buf) {
    for(uint16_t i = 0; i < len; ++i) {
        mmap_write_noexec(obj, addr + i, buf[i]);
    }
}

uint8_t *mmap_debug_read_block(mmap_t *obj, uint16_t addr, uint16_t len) {
    static uint8_t tempbuffer[16][256];
    static int bufferindex;

    if(len > 255) {
        return NULL;
    }

    uint8_t *returnbuffer = tempbuffer[bufferindex];
    mmap_read_block_noexec(obj, addr, len, returnbuffer);
    bufferindex = (bufferindex + 1) % 16;

    return returnbuffer;
}

/*
 *  Statics
 *
 */

_memnode_t *mmap_create_memnode(_memnode_t *prev, _memnode_t *next, const memspace_t *elem) {
    _memnode_t *newnode = malloc(sizeof(_memnode_t));
    if(!newnode) {
        return NULL;
    }

    memcpy(&newnode->elem, elem, sizeof(memspace_t));
    if(newnode->elem.flags & MMAP_ALLOC) {
        newnode->elem.memory = malloc(newnode->elem.size);
        if(!newnode->elem.memory) {
            free(newnode);
            return NULL;
        }
    }

    if(prev) {
        prev->next = newnode;
    }

    newnode->next = next;

    return newnode;
}