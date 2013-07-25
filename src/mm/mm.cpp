#include "os.h"
#include "gdtdef.h"
#include "mmdef.h"

#define MAX_SIZE_T           (~(size_t)0)
#define MFAIL                ((void*)(MAX_SIZE_T))

u32 mmKernelPoolBase; 
u32 mmSbrkTop;
u32 mmKernelPoolEnd; 
u32 mmKernelPoolSize = KERNEL_POOL_SIZE;
u32 mmPhysMemEnd; 
u32 mmPhysBlockCount;
u32 mmPhysBlockFree;
u32 mmPhysPoolBase;
u32 mmPhysPoolEnd;



BLOCK_ALLOCATOR mmPhysBlockAllocator;
BLOCK_ALLOCATOR mmSharedBlockAllocator;
BLOCK_ALLOCATOR mmIoBlockAllocator;

PAGE_DIR *mmKernelPageDir;
PAGE_TABLE *mmKernelPageTables;


extern "C" void keSetGDT(GDTR *gr);

void mmLoadMmap() {
	extern MULTIBOOT_INFO	*mbd;
	MMAP_ENTRY *pmap;
	
	pmap = (MMAP_ENTRY*)(mbd->mmap_addr);
	kprintf("low: %dk, upper: %dk\n", mbd->mem_lower, mbd->mem_upper);
	kprintf("pmap: %08x, mmap_length: %d\n", pmap, mbd->mmap_length);
	while((u32)pmap < mbd->mmap_addr + mbd->mmap_length) {
		kprintf("size: %d, addr: %x'%08x, len: %x'%08x, type: %d\n", pmap->size, pmap->baseH, pmap->baseL, pmap->lengthH, pmap->lengthL, pmap->type);
		pmap = (MMAP_ENTRY*)((u32)pmap + pmap->size + 4);
	}
}	

void* mmSbrkForMalloc(int32_t size) {

	void * ptr = 0;
	
	kprintf("sbrk: %d\n", size);
	if (size > 0) {
	  ptr = (void*) mmSbrkTop;
      if ((mmSbrkTop + size) >= mmKernelPoolEnd) return (void *) MFAIL;
      mmSbrkTop = mmSbrkTop + size;
      return ptr;
    }
    else if (size < 0) {
      return (void *) MFAIL;
    }
    else {
      return (void *) mmSbrkTop;
    }
}

void mmInitPool() {
	extern MULTIBOOT_INFO	*mbd;
	extern u32 end;
	u32 low = mbd->mem_lower + 1;
	u32 high = mbd->mem_upper - 1;
	low += 1; high -= 1;
	assert(low < (PHYSMEM_MAX / 1024));
	low = low * 1024;
	if (low <= ((u32)(&end))) low = (u32)(&end) + 1;
	if (high > (PHYSMEM_MAX / 1024)) {
		high = PHYSMEM_MAX;
	} else {
		high = high * 1024;
	}
	low = ((low - 1) / BLOCK_SIZE + 1) * BLOCK_SIZE;
	high = ((high + 1) / BLOCK_SIZE - 1) * BLOCK_SIZE;
	assert(low < high);
	mmKernelPoolBase = low;
	mmKernelPoolEnd = low + KERNEL_POOL_SIZE;
	assert(mmKernelPoolEnd < high);
	mmPhysPoolBase = mmKernelPoolEnd;
	mmPhysPoolEnd = high;
	assert(mmPhysPoolBase < mmPhysPoolEnd);
	mmPhysBlockCount = (mmPhysPoolEnd - mmPhysPoolBase) / BLOCK_SIZE;
	mmSbrkTop = mmKernelPoolBase;
	kprintf("kernelPoolBase: %08x, End: %08x\n", mmKernelPoolBase, mmKernelPoolEnd);
	kprintf("physPoolBase: %08x, End: %08x\n", mmPhysPoolBase, mmPhysPoolEnd);
	assert(isOk(mmInitBlockAllocator(&mmPhysBlockAllocator, mmPhysPoolBase, mmPhysBlockCount)));
	assert(isOk(mmInitBlockAllocator(&mmSharedBlockAllocator, SHAREDMEM_START, SHAREDMEM_BLOCK_COUNT)));
	assert(isOk(mmInitBlockAllocator(&mmIoBlockAllocator, IOMEM_START, IOMEM_BLOCK_COUNT)));
	
	mmKernelPageDir = (PPAGE_DIR)memalign(sizeof(PAGE_DIR), sizeof(PAGE_DIR));
	memset(mmKernelPageDir, 0 ,sizeof(PAGE_DIR));
	kprintf("kernelPageDir: %08x\n", mmKernelPageDir);

}		

 
void mmInitStage0() {
	extern GDT_ENTRY GDT[]; 
	GDTR gr;
	GDT[0] = 0;
	GDT[1] = create_descriptor(0, 0x0007ffff, (GDT_CODE_PL0));
	GDT[2] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
	GDT[3] = 0;
	GDT[4] = 0;
//    GDT[3] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
//    GDT[4] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
    GDT[5] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL3));
	GDT[6] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL3));
	gr.base = (u32) GDT;
	gr.limit = 7*8 - 1;
	keSetGDT(&gr);
	kprintf("GDTBase : %x\n", GDT);
	

	mmInitPool();

}

