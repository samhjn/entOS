#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;
typedef u32 size_t;
#undef WIN32
#undef _WIN32
#define assert(e)       ((e) ? (void)0 : dbgAssert((char*)#e, (char*)__FILE__, (int)__LINE__))

#define RESULT_SUCCESS	0
#define RESULT_ERROR	2
#define RESULT_ERROR_PARAM	3
#define RESULT_ERROR_NOMEMORY 4

#define STDC	extern "C"


#define isOk(a)  ((a) == RESULT_SUCCESS)


extern "C" {
typedef	u64 GDT_ENTRY;

typedef struct {
	u32 edi,esi,ebp,esp,ebx,edx,ecx,eax;
} REGS_PUSHAD;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) GDTR, IDTR;

typedef struct {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} __attribute__((packed)) IDT_ENTRY;


}

#include "multiboot.h"

