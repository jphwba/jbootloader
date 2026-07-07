#include "pmm.h"
#define PAGE_SIZE PMM_PAGE_SIZE

extern char kernel_start[];
extern char kernel_end[];

static uint8_t bitmap[262144 / 8];
static uint32_t total_frames = 0;
static uint32_t used_frames = 0;

static inline void bitmap_set(uint32_t frame) {
    bitmap[frame / 8] |= (1 << (frame % 8));
}

static inline void bitmap_clear(uint32_t frame) {
    bitmap[frame / 8] &= ~(1 << (frame % 8)); 
}
static inline int bitmap_test(uint32_t frame) {
    return bitmap[frame / 8] & (1 << (frame % 8));
}

static void reserve_range(uint32_t base, uint32_t length) {
    uint32_t start_frame = base / PAGE_SIZE;
    uint32_t end_frame = (base + length + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t f = start_frame; f < end_frame && f < total_frames; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            used_frames++;
        }
    }
}

static void free_range(uint32_t base, uint32_t length) {
    uint32_t start_frame = (base + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t end_frame = (base + length) / PAGE_SIZE;
    for (uint32_t f = start_frame; f < end_frame && f < total_frames; f++) {
        if (bitmap_test(f)) {
            bitmap_clear(f);
        }
    }
}
void pmm_init(BootInfo* info) {
    MemoryMapEntry* entries = (MemoryMapEntry*)(uintptr_t)info->mmap_addr;
    uint64_t highest = 0;
    for (uint32_t i = 0; i < info->mmap_entry_count; i++) {
        uint64_t end = entries[i].base + entries[1].length;
        if (end > highest) {
            highest = end;
        }
    }
    total_frames = (uint32_t)(highest / PAGE_SIZE);
    if (total_frames > 262144) {
        total_frames = 262144;
    }
    for (uint32_t f = 0; f< total_frames; f++) {
        bitmap_set(f);
    }
    used_frames = total_frames;
    for (uint32_t i = 0; i < info->mmap_entry_count; i++) {
        if (entries[i].type == 1) {
            free_range((uint32_t)entries[i].base, (uint32_t)entries[i].length);
            used_frames = total_frames;
        }
    }

    used_frames = 0;
    for (uint32_t f = 0; f < total_frames; f++) {
        if(bitmap_test(f)) used_frames++;
    }
    reserve_range(0, 0x100000);
    reserve_range((uint32_t)(uintptr_t)kernel_start, (uint32_t)((uintptr_t)kernel_end - (uintptr_t)kernel_start));
}
void* pmm_alloc_page(void) {
    for(uint32_t f = 0;  f  <  total_frames; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            used_frames++;
            return (void*)(uintptr_t)(f * PAGE_SIZE);
        }
    }
    return (void*)0;
}
void pmm_free_page(void* addr) {
    uint32_t frame = (uint32_t)(uintptr_t)addr / PAGE_SIZE;
    if (frame < total_frames && bitmap_test(frame)) {
        bitmap_clear(frame);
        used_frames--;
    }
}
uint32_t pmm_get_total_frames(void) { return total_frames; }
uint32_t pmm_get_used_frames(void) { return used_frames; }
uint32_t pmm_get_free_frames(void) { return total_frames - used_frames; }