// FAT16 file manager

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a" (val), "Nd" (port));
}
// inw
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}
// lets use ATA
void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    outb(0x1F6, (lba << 24) | 0xE0);
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);
    while (!(inb(0x1F7) & 0x08));
    for (int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = inw(0x1F0);
    }
}

typedef struct {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t ctime_ms;
    uint16_t ctime;
    uint16_t cdate;
    uint16_t ldate;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) FAT16_DirEntry;
// beta
void fileman_list_root() {
    uint16_t sector_buffer[512];
    ata_read_sector(19, (uint8_t*)sector_buffer);
    FAT16_DirEntry* entries = (FAT16_DirEntry*)sector_buffer;

}