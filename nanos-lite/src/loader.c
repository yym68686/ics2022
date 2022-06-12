#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x8048000)
void ramdisk_read(void *, uint32_t, uint32_t);
size_t get_ramdisk_size();
void *new_page();

uintptr_t loader(_Protect *as, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    size_t len = fs_filesz(fd);
    Log("LOAD [%d] %s. Size:%d", fd, filename, len);

    void *fz_end = DEFAULT_ENTRY + len;
    void *va, *pa;
    for(va = DEFAULT_ENTRY; va < fz_end; va += PGSIZE){
        pa = new_page();
        _map(as, va, pa);
        fs_read(fd, pa, (fz_end - va) < PGSIZE ? (fz_end - va) : PGSIZE);
        Log("va: 0x%08x, pa: 0x%08x", va, pa);
    }
    return (uintptr_t)DEFAULT_ENTRY;
}
