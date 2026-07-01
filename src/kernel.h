#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>

typedef struct {
    uint32_t boot_drive;
    uint32_t mmap_entry_count;
    uint32_t mmap_addr;
    uint32_t kernel_size;
} BootInfo;

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_ext;
} _attribute_((packed)) MemoryMapEntry;

void kernel_main(BootInfo* info);

#endif