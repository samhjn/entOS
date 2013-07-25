#include "os.h"
#define MARK_USED(blk)  (pb->buffer[(blk)/32] |= (1 << ((blk) % 32)))
#define MARK_UNUSED(blk) (pb->buffer[(blk)/32] -= (1 << ((blk) % 32)))
#define IS_USED(blk) (pb->buffer[(blk)/32] & (1 << ((blk) % 32)))

int mmInitBlockAllocator(PBLOCK_ALLOCATOR pb, u32 base, u32 blockCount) {
	u32* ptr;
	u32 bufSize = (blockCount / 32 + 1) * sizeof(u32); 
	
	pb->baseAddr = base;
	pb->endAddr = base + blockCount * BLOCK_SIZE; 
	pb->blockCount = blockCount;
	ptr = (u32*) malloc(bufSize);
	if (!ptr) return RESULT_ERROR_NOMEMORY;
	pb->buffer = ptr;
	memset(ptr, 0, bufSize);
	pb->freeCount = blockCount;
	pb->bufSize = bufSize;
	return RESULT_SUCCESS;
}

 
void* mmAllocateBlock(PBLOCK_ALLOCATOR pb, int blockCount, void* base) {
	int i, j, k, count = 0;
	u32 t;
	u32 blkStart, blkEnd;
	void* ptr = 0;
	
	assert(blockCount > 0);
	if (base) {
		assert((u32)base % BLOCK_SIZE == 0);
		assert((u32)base >= pb->baseAddr);
		blkStart = ((u32)base - pb->baseAddr) / BLOCK_SIZE;
		blkEnd = blkStart + blockCount;
		if (blkEnd >= pb->blockCount) goto final;
		for (i = blkStart; i < blkEnd; i++) if (IS_USED(i)) goto final;
		for (k = blkStart; k < blkEnd; k++) MARK_USED(k);
		pb->freeCount -= blockCount;
		ptr = (void*) (pb->baseAddr + blkStart * BLOCK_SIZE);
		goto final;
	}
	for (i = 0; i < pb->bufSize; i++) {
		if (pb->buffer[i] != 0xffffffff) {
			t = pb->buffer[i];
			for (j = 0; j < 32;j ++) if (!(t & (1 << j))) { //t[j] == 0
				count ++;
				if (count == blockCount) {
					blkEnd = (i * 32 + j) + 1;
					blkStart = blkEnd - count;
					if (blkEnd >= pb->blockCount) goto final;
					for (k = blkStart; k < blkEnd; k++) MARK_USED(k);
					pb->freeCount -= blockCount;
					ptr = (void*) (pb->baseAddr + blkStart * BLOCK_SIZE);
					goto final;
				}
			} else {
				count = 0;
			}
		}
	}
final:
	return ptr;
} 

void mmFreeBlock(PBLOCK_ALLOCATOR pb, void* ptr, int blockCount) {
	u32 blk;
	int i;
	assert(ptr);
	assert((u32)ptr % BLOCK_SIZE == 0);
	blk = ((u32)ptr - pb->baseAddr) / BLOCK_SIZE;
	for (i = blk; i < blk + blockCount; i++) {
		assert(IS_USED(i));
		MARK_UNUSED(i);
	}
}
