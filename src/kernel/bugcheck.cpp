#include "os.h"

void dbgPrintRegs(REGS_PUSHAD r) {
	kprintf("EAX: %08x EBX: %08x ECX: %08x EDX:%08x\n", r.eax,r.ebx,r.ecx,r.edx);
	kprintf("ESP: %08x EBP: %08x ESI: %08x EDI: %08x\n", r.esp, r.ebp, r.esi, r.edi);
	
}

void dbgAssert(char* e, char* file, int line) {
	kprintf("Assert failed\n");
	kprintf("File: %s, Ln: %d, Equ: %s\n", file, line, e);
	kprintf("System halted.\n");
	while(1);
}

void keBugCheck(char* reason, u32 param1, u32 param2, u32 param3, u32 param4) {
	keEnterCli();
	kprintf("Bugcheck: %s\n", reason);
	kprintf("%08x %08x %08x %08x\n", param1, param2, param3, param4);
	kprintf("System halted.\n");
	while(1);
	keLeaveCli();
}