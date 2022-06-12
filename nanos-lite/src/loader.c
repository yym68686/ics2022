#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x8048000)
void ramdisk_read(void *, uint32_t, uint32_t);
size_t get_ramdisk_size();
void *new_page();

uintptr_t loader(_Protect *as, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    size_t f_size = fs_filesz(fd);
    uint32_t vaddr, paddr;
	uint32_t endaddress = (uint32_t)DEFAULT_ENTRY + f_size;

    //进行分页映射并加载到内存位置0x8048000附近
    for (vaddr = (uint32_t)DEFAULT_ENTRY; vaddr < endaddress; vaddr += PGSIZE) {
        paddr = (uint32_t)new_page();
        Log("Map va to pa: 0x%08x to 0x%08x", vaddr, paddr);
        _map(as, (void *)vaddr, (void *)paddr);
        fs_read(fd, (void *)paddr, (endaddress - vaddr) < PGSIZE ? endaddress : PGSIZE);
    }

    fs_close(fd);

    return (uintptr_t)DEFAULT_ENTRY;
}
