#define	PAGE_SIZE	4096U
#define PAGES_IN_BLOCK	64U
#define	BLOCK_SIZE 	(PAGE_SIZE * PAGES_IN_BLOCK)
#define BLOCKS_FOR_KERNEL_POOL	128U
#define KERNEL_POOL_SIZE	(BLOCKS_FOR_KERNEL_POOL * BLOCK_SIZE)
#define PHYSMEM_MAX	0x6fffffff
#define SHAREDMEM_START 0x70000000
#define SHAREDMEM_END 0x77ffffff
#define SHAREDMEM_BLOCK_COUNT 512
#define IOMEM_START 0x78000000
#define IOMEM_END 0x7fffffff
#define IOMEM_BLOCK_COUNT 512
#define USERMEM_START 0x80000000
#define USERMEM_END 0xefffffff

void mmInitStage0();
extern "C" {
void* malloc(size_t);
void  free(void*);
void* memalign(size_t, size_t);
}

/*-----defines for dlmalloc-----*/
#define EINVAL	RESULT_ERROR_PARAM
#define ENOMEM 	RESULT_ERROR_NOMEMORY
#define LACKS_SYS_TYPES_H
#define MORECORE mmSbrkForMalloc
#define ABORT dbgAssert((char*)"dlmalloc" , (char*)__FILE__, __LINE__)
#define malloc_getpagesize BLOCK_SIZE
//#define USE_LOCKS 1
#define USE_SPIN_LOCKS 0
/*
#define MLOCK_T               u32
#define ACQUIRE_LOCK(lk)      (keEnterCrit(), 0)
#define RELEASE_LOCK(lk)      keLeaveCrit()
#define TRY_LOCK(lk)          TryEnterCriticalSection(lk)
#define INITIAL_LOCK(lk)      (!InitializeCriticalSectionAndSpinCount((lk), 0x80000000|4000))
#define DESTROY_LOCK(lk)      (DeleteCriticalSection(lk), 0)
#define NEED_GLOBAL_LOCK_INIT
*/
/*-----defines for dlmalloc-----*/

typedef struct {
	u32 baseAddr;
	u32 endAddr;
	u32 blockCount;
	u32 *buffer;
	u32 freeCount;
	u32 bufSize;
} BLOCK_ALLOCATOR, *PBLOCK_ALLOCATOR;

void mmFreeBlock(PBLOCK_ALLOCATOR pb, void* ptr, int blockCount) ;
void* mmAllocateBlock(PBLOCK_ALLOCATOR pb, int blockCount, void* base);
int mmInitBlockAllocator(PBLOCK_ALLOCATOR pb, u32 base, u32 blockCount);
void* mmSbrkForMalloc(int32_t size) ;