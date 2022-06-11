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

uint32_t vaddr_read(vaddr_t addr, int len) {
    if(cpu.cr0.paging) {
        //跨页访存.
        if ((addr & 0xfff) + len - 1 > 0xfff) {
            uint32_t val = 0;
            int i = 0;
            for (; i < len; i++)
                val += (1 << (8 * i)) * (uint8_t)paddr_read(page_translate(addr + i, false), 1);
            return val;
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
    if(cpu.cr0.paging) {
        //跨页访存.
        if ((addr & 0xfff) + len - 1 > 0xfff)
            assert(0);
        else {
            paddr_t paddr = page_translate(addr, true);
            return paddr_write(paddr, len, data);
        }
    }
    else
        paddr_write(addr, len, data);
}
