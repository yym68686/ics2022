#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x8048000)
void ramdisk_read(void *, uint32_t, uint32_t);
size_t get_ramdisk_size();
void *new_page();

uintptr_t loader(_Protect *as, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    int filesize = fs_filesz(fd);
    void *pa;
    void *va = DEFAULT_ENTRY;
    while(filesize > 0) {
        pa = new_page();
        _map(as, va, pa);
        va += PGSIZE;
        fs_read(fd, pa, PGSIZE);
        filesize -= PGSIZE;
    }
    return (uintptr_t)DEFAULT_ENTRY;
}
