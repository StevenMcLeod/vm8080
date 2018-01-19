//
// Created by Steven McLeod on 2017-11-07.
//

#ifndef VM8080_VM_H
#define VM8080_VM_H

#include <stdint.h>
#include "cpu.h"

int vm_init(cpu_t *obj);
int vm_reset(cpu_t *obj);
int vm_destroy(cpu_t *obj);
int vm_run(cpu_t *obj);
int vm_run_rt(cpu_t *obj);
int vm_debug(cpu_t *obj);
int vm_singlestep(cpu_t *obj);

int vm_loadrom(cpu_t *obj, uint16_t origin, uint16_t len, uint8_t *prog);
int vm_loadram(cpu_t *obj, uint16_t origin, uint16_t len, uint8_t *prog);

int vm_loadrom_file(cpu_t *obj, uint16_t origin, uint16_t len, const char *fn);

int vm_loadio(cpu_t *obj, const memspace_t *space);
unsigned vm_loadio_arr(cpu_t *obj, const memspace_t *space, unsigned len);

#endif //VM8080_VM_H
