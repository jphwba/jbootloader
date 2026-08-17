#ifndef FAT16_H
#define FAT16_H
#include <stdint.h>

int fat16_init(void);
typedef void (*fat16_list_cb)(const char* name, uint32_t size);
void fat16_list(fat16_list_cb cb);
uint32_t fat16_read_file(const char* name, void* buffer, uint32_t max_size);

#endif