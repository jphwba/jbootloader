#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#define PAGE_PRESENT 0x1u
#define PAGE_RW 0x2u
#define PAGE_USER 0x4u

void paging_init(void);
void paging_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void paging_identity_map_range(uint32_t start, uint32_t end);
#endif