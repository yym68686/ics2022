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

paddr_t page_translate(vaddr_t addr, bool flag){
    paddr_t paddr = addr;
    /* only when protect mode and paging mode enable translate*/
    if(cpu.cr0.protect_enable && cpu.cr0.paging){
        /* initialize */
        paddr_t pdeptr, pteptr;
        PDE pde;
        PTE pte;
        /* find pde and read */
        pdeptr = (paddr_t)(cpu.cr3.page_directory_base << 12) | (paddr_t)(((addr >> 22) & 0x3ff) * 4);
        pde.val = paddr_read(pdeptr, 4);
        assert(pde.present);
        pde.accessed = 1;
        /* find pte and read */
        pteptr = (paddr_t)(pde.page_frame << 12) | (paddr_t)(((addr >> 12) & 0x3ff) * 4);
        pte.val = paddr_read(pteptr, 4);
        assert(pte.present);
        pte.accessed = 1;
        pte.dirty = (flag == true) ? 1 : 0;
        /* find page and read */
        paddr = (paddr_t)(pte.page_frame << 12) | (paddr_t)(addr & 0xfff);
    }
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
