#include "fat16.h"
#include "ata.h"
#include "heap.h"
#include <stddef.h>
#include <stdint.h>
#include "printf.h"

#define FS_START_LBA 109
#define SECTOR_SIZE 512
#define ATTR_LONG_NAME 0x0F
#define ATTR_DIRECTORY 0x10
#define ATTR_VOLUME_ID 0x08

typedef struct {
    uint8_t name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t first_cluster_hi;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_lo;
    uint32_t file_size;
} __attribute__((packed)) fat_dirent_t;

static uint16_t bytes_per_sector;
static uint8_t sectors_per_cluster;
static uint16_t reserved_sectors;
static uint8_t num_fats;
static uint16_t root_entry_count;
static uint16_t sectors_per_fat;
static uint32_t fat_start_lba;
static uint32_t root_dir_start_lba;
static uint32_t root_dir_sectors;
static uint32_t data_start_lba;
static int mounted = 0;
static uint16_t rd_u16(const uint8_t*b, int off){
    return (uint16_t)(b[off] | (b[off + 1] << 8));
}

int fat16_init(void) {
    uint8_t boot[SECTOR_SIZE];
    int rc = ata_read_sectors(FS_START_LBA, 1, boot);
    kprintf("fat16: ata_read_sectors rc=%d sig=%x%x b0=%x b1=%x b2=%x\n",
            rc, boot[510], boot[511], boot[0], boot[1], boot[2]);
    if (rc != 0) {
        return -1;
    }
    if (boot[510] != 0x55 || boot[511] != 0xAA) {
        return -1; /* no boot signature, not a FAT volume at all */
    }

    bytes_per_sector = rd_u16(boot, 11);
    sectors_per_cluster = boot[13];
    reserved_sectors = rd_u16(boot, 14);
    num_fats = boot[16];
    root_entry_count = rd_u16(boot, 17);
    sectors_per_fat = rd_u16(boot, 22);
    if (bytes_per_sector != SECTOR_SIZE || sectors_per_fat == 0 || sectors_per_cluster == 0) {
        return -1;
    }
    fat_start_lba = FS_START_LBA + reserved_sectors;
    root_dir_start_lba = fat_start_lba + (uint32_t)num_fats * sectors_per_fat;
    root_dir_sectors = ((uint32_t)root_entry_count * 32 + SECTOR_SIZE - 1) / SECTOR_SIZE;
    data_start_lba = root_dir_start_lba + root_dir_sectors;
    mounted = 1;
    return 0;
}

static void name_83_to_display(const uint8_t* raw, char* out) {
    int p = 0;
    for(int i = 0; i < 8 && raw[i] != ' '; i++) {
        out[p++] = (char)raw[i];
    }
    if (raw[8] != ' ') {
        out[p++] = '.';
        for (int i = 8; i < 11 && raw[i] != ' '; i++) {
            out[p++] = (char)raw[i];
        }
    }
    out[p] = 0;
}
static void name_display_to_83(const char* name, uint8_t* out) {
    for (int i = 0; i < 11; i++) {
        out[i] = ' ';
    }
    int i = 0, p = 0;
    while (name[i] && name[i] != '.' && p < 8) {
        char c = name[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        out[p++] = (uint8_t)c;
        i++;
    }
    while (name[i] && name[i] != '.') i++;
    if (name[i] == '.') {
        i++;
        p = 8;
        while (name[i] && p < 11) {
            char c = name[i];
            if(c >= 'a' && c <= 'z') c = (char)(c - 32);
            out[p++] = (uint8_t)c;
            i++;
        }
    }
}
void fat16_list(fat16_list_cb cb) {
    if (!mounted || !cb) {
        return;
    }
    uint8_t sector[SECTOR_SIZE];
    uint32_t entries_per_sector = SECTOR_SIZE / sizeof(fat_dirent_t);
    for (uint32_t s = 0; s < root_dir_sectors; s++) {
        if (ata_read_sectors(root_dir_start_lba + s, 1, sector) != 0) {
            return;
        }
        fat_dirent_t* entries = (fat_dirent_t*)sector;
        for (uint32_t i = 0; i < entries_per_sector; i++) {
            fat_dirent_t* e = &entries[i];
            if (e->name[0] == 0x00) {
                return;
            }
            if (e->name[0] == 0xE5) {
                continue;
            }
            if (e->attr == ATTR_LONG_NAME || (e->attr & ATTR_VOLUME_ID) || (e->attr & ATTR_DIRECTORY)) {
                continue;
            }
            char display [13];
            name_83_to_display(e->name, display);
            cb(display, e->file_size);
        }
    }
}
uint32_t fat16_read_file(const char* name, void* buffer, uint32_t max_size) {
    if (!mounted) {
        return 0;
    }
    uint8_t want[11];
    name_display_to_83(name, want);
    uint8_t sector[SECTOR_SIZE];
    uint32_t entries_per_sector = SECTOR_SIZE / sizeof(fat_dirent_t);
    uint16_t found_cluster = 0;
    uint32_t found_size = 0;
    int found = 0;

    for (uint32_t s = 0; s < root_dir_sectors && !found; s++) {
        if (ata_read_sectors(root_dir_start_lba + s, 1, sector) !=0) {
            return 0;
        }
        fat_dirent_t* entries = (fat_dirent_t*)sector;
        for(uint32_t i = 0; i < entries_per_sector; i++) { 
            fat_dirent_t* e = &entries[i];
            if (e->name[0] == 0x00) break;
            if (e->name[0] == 0xE5) continue;
            if (e->attr == ATTR_LONG_NAME || (e->attr & ATTR_VOLUME_ID) || (e->attr & ATTR_DIRECTORY)) continue;
            int match = 1;
            for (int k = 0; k < 11; k++) {
                if (e->name[k] != want[k]) {match = 0; break;}
            }
            if (match) {
                found_cluster = e->first_cluster_lo;
                found_size = e->file_size;
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        return 0;
    }
    uint8_t* out = (uint8_t*)buffer;
    uint32_t total_written = 0;
    uint16_t cluster = found_cluster;
    uint32_t cluster_bytes = (uint32_t)sectors_per_cluster * SECTOR_SIZE;
    while (cluster >= 2 && cluster < 0xFFF8 && total_written < found_size && total_written < max_size) {
        uint32_t lba = data_start_lba + (uint32_t)(cluster - 2) * sectors_per_cluster;
        uint32_t remaining_file = found_size - total_written;
        uint32_t remaining_buf = max_size - total_written;
        uint32_t to_copy = cluster_bytes;
        if (to_copy > remaining_file) to_copy = remaining_file;
        if (to_copy > remaining_buf) to_copy = remaining_buf; 
        if (to_copy == cluster_bytes) {
            if (ata_read_sectors(lba, sectors_per_cluster, out + total_written) != 0) {
                break;
            }
        } else {
            uint8_t* scratch = (uint8_t*)kmalloc(cluster_bytes);
            if (!scratch) {
                break;
            }
            if (ata_read_sectors(lba, sectors_per_cluster, scratch) != 0) {
                kfree(scratch);
                break;
            }
            for(uint32_t b = 0; b < to_copy; b++) {
                out[total_written + b] = scratch[b];
            }
            kfree(scratch);
        }
        total_written += to_copy;
        
        uint32_t fat_offset = (uint32_t)cluster * 2;
        uint32_t fat_sector = fat_start_lba + (fat_offset / SECTOR_SIZE);
        uint32_t fat_sector_offset = fat_offset % SECTOR_SIZE;
        uint8_t fat_sec[SECTOR_SIZE];
        if (ata_read_sectors(fat_sector, 1, fat_sec) != 0) {
            break;
        }
        cluster = rd_u16(fat_sec, (int)fat_sector_offset);
    }
    return total_written;
}
