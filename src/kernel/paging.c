#include "paging.h"
#include "pmm.h"
#define PAGE_DIR_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_SIZE 4096u
static uint32_t* page_directory = 0;

void paging_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint32_t dir_index = virt_addr >> 22;
    uint32_t table_index = (virt_addr >> 12) & 0x3FF;
    if (!(page_directory[dir_index] & PAGE_PRESENT)) {
        uint32_t* table = (uint32_t*)pmm_alloc_page();
        for(int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            table[i] = 0;
        }
        page_directory[dir_index] = ((uintptr_t)table) | PAGE_PRESENT | PAGE_RW;
    }
    uint32_t* table = (uint32_t*)(uintptr_t)(page_directory[dir_index] & ~0xFFFu);
    table[table_index] = (phys_addr & ~0xFFFu) | (flags & 0xFFFu) | PAGE_PRESENT;
}

void paging_identity_map_range(uint32_t start, uint32_t end) {
    start &= ~0xFFFU;
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        paging_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }
}

void paging_init(void) {
    page_directory = (uint32_t*)pmm_alloc_page();
    for (int i = 0; i < PAGE_DIR_ENTRIES; i++) {
        page_directory[i] = 0;
    }
    paging_identity_map_range(0x0, 0x1000000);
    __asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory) : "memory");
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0) : "memory");
}
