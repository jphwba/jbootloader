#include "heap.h"
#include "pmm.h"
#include <stdint.h>
#define HEAP_PAGES 64

typedef struct block_header {
    size_t size;
    int free;
    struct block_header* next;
} block_header_t;
static block_header_t* free_list = 0;
void heap_init(void) {
    void* base = pmm_alloc_pages(HEAP_PAGES);
    if (!base) {
        free_list = 0;
        return;
    }
    uint32_t size = HEAP_PAGES * PMM_PAGE_SIZE;
    free_list = (block_header_t*)base;
    free_list->size = size - sizeof(block_header_t);
    free_list->free = 1;
    free_list->next = 0;
}

void* kmalloc(size_t size) {
    size = (size + 7u) & ~((size_t)7u);
    block_header_t* current = free_list;

    while(current) {
        if(current->free && current->size >= size) {
            if(current->size >= size + sizeof(block_header_t) + 8) {
                block_header_t* remainder = (block_header_t*)((uint8_t*)current + sizeof(block_header_t) + size);
                remainder->size = current->size - size - sizeof(block_header_t);
                remainder->free = 1;
                remainder->next = current->next;
                current->size = size;
                current->next = remainder;
            }
            current->free = 0;
            return (void*)((uint8_t*)current + sizeof(block_header_t));
        }
        current = current->next;
    }
    return 0;
}

void kfree(void* ptr) {
    if(!ptr) {
        return;
    }
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    block->free = 1;

    block_header_t* current = free_list;
    while(current && current->next) {
        uint8_t* end_of_current = (uint8_t*)current + sizeof(block_header_t) + current->size;
        if(current->free && current->next->free && end_of_current == (uint8_t*)current->next) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}