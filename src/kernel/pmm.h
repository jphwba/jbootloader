#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include <stddef.h>
#include "../kernel.h"
#define PMM_PAGE_SIZE 4096u

void pmm_init(BootInfo* info);
void * pmm_alloc_page(void);
void pmm_free_page(void* addr);
uint32_t pmm_get_total_frames(void);
uint32_t pmm_get_used_frames(void);
uint32_t pmm_get_free_frames(void);
#endif