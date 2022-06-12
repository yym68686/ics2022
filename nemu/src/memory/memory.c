#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1)
    return mmio_read(addr, len, mmio_id);
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1)
    mmio_write(addr, len, data, mmio_id);
  else
    memcpy(guest_to_host(addr), &data, len);
}

paddr_t page_translate(vaddr_t vaddr, bool flag) {
    PDE page_dir_item;
    PTE page_table_item;
    //页目录项的地址 = 页目录表基地址 + (页目录表索引) * 4
    paddr_t page_dir_item_addr = (cpu.cr3.page_directory_base << 12) + ((vaddr >> 22) & 0x3ff) * 4;
    //读取页目录项
    page_dir_item.val = paddr_read(page_dir_item_addr, 4);
    //验证present位
    assert(page_dir_item.present);
    //根据讲义，accessed为1，若为1表示该页被CPU访问过，由CPU置1，由操作系统清0
    page_dir_item.accessed = 1;
    //写回到页目录项所在地址
    paddr_write(page_dir_item_addr, 4, page_dir_item.val);
    //页表项的地址 = 页目录表项对应的页表基址 + 页表索引 * 4
    paddr_t page_table_item_addr = (page_dir_item.page_frame << 12) + ((vaddr >> 12) & 0x3ff) * 4;
    //读取页表项
    page_table_item.val = paddr_read(page_table_item_addr, 4);
    //验证present位
    assert(page_table_item.present);
    page_table_item.accessed = 1;
    //如果是写操作，脏位设为1，当CPU对一个页执行写操作时，设置对应页表项的D位为1，此项仅针对页表项有效，并不会修改页目录表项的D位
    if (flag) page_table_item.dirty = 1;
    //写回到页表项所在地址
    paddr_write(page_table_item_addr, 4, page_table_item.val);
    paddr_t paddr = (page_table_item.page_frame << 12) + (vaddr & 0xfff);
    //Log("vaddr: %#10x, paddr: %#10x", vaddr, paddr);
    return paddr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	paddr_t paddr;
    if(cpu.cr0.paging) {
        //跨页访存 0x1000 = 1000000000000 = 2^12 = 4KB
        if ((addr & 0xfff) + len > 0x1000) {
			union {
			  uint8_t bytes[4];
			  uint32_t dword;
			} data = {0};
			for (int i = 0; i < len; i++) {
			  paddr = page_translate(addr + i, false);
			  data.bytes[i] = (uint8_t)paddr_read(paddr, 1);
			}
			return data.dword;
        }
        else {
            paddr_t paddr = page_translate(addr, false);
            return paddr_read(paddr, len);
        }
    }
    else
        return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
	paddr_t paddr;
    if(cpu.cr0.paging) {
        //跨页访存
        if ((addr & 0xfff) + len > 0x1000)
		    for (int i = 0; i < len; i++) {
			  paddr = page_translate(addr + i, true);
			  paddr_write(paddr, 1, data);
			  data >>= 8;
			}
        else {
            paddr_t paddr = page_translate(addr, true);
            return paddr_write(paddr, len, data);
        }
    }
    else
        paddr_write(addr, len, data);
}
