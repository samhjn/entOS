#include "os.h"

typedef void (*FnIrqHandler)();
FnIrqHandler keIrqHandler[16] ;

extern "C" {
	 void keExHandler0();
	 void keExHandler1();
	 void keExHandler2();
	 void keExHandler3();
	 void keExHandler4();
	 void keExHandler5();
	 void keExHandler6();
	void keExHandler7();
	void keExHandler8();
	void keExHandler9();
	void keExHandler10();
	void keExHandler11();
	void keExHandler12();
	void keExHandler13();
	void keExHandler14();
	void keExHandler16();
	
	void keIrqHandler0();
	void keIrqHandler1();
	void keIrqHandler2();
	void keIrqHandler3();
	void keIrqHandler4();
	void keIrqHandler5();
	void keIrqHandler6();
	void keIrqHandler7();
	void keIrqHandler8();
	void keIrqHandler9();
	void keIrqHandler10();
	void keIrqHandler11();
	void keIrqHandler12();
	void keIrqHandler13();
	void keIrqHandler14();
	void keIrqHandler15();
	extern IDT_ENTRY IDT[256];
}

STDC
void keDispatchException(
					REGS_PUSHAD regs, 
					u32 vecno, 
					u32 errcode,
					u32 eip,
					u32 cs,
					u32 eflags, 
					u32 esp, 
					u32 ss) {
	keEnterCli();
	kprintf("\nException: %d\nErrCode: %d, EIP: %08x, CS: %04x, EFLAGS: %08x\n", vecno, errcode, eip, cs, eflags);
	dbgPrintRegs(regs);
	kprintf("System halted.");
	while(1);
	keLeaveCli();
}

STDC
void keDispatchIrq(
					REGS_PUSHAD regs, 
				    u32 irq, 
					u32 eip,
					u32 cs,
					u32 eflags, 
					u32 esp, 
					u32 ss) {
					
	FnIrqHandler ptr;
	
	REQUIRE_TASK_LEVEL;
	kpc->cliCount ++; 
	kpc->exLevel = CLI_LEVEL;
	
	ptr = keIrqHandler[irq];
	if (ptr == 0) {
		keBugCheck("unhandled irq", irq, 0 , 0, 0);
	}
	ptr();
	kePICDoEoi(irq);
	
	kpc->cliCount --;
	assert(kpc->cliCount == 0);
	kpc->exLevel = TASK_LEVEL;
}

void keSetIrqHandler(int irq, void* pHandler) {
	assert(keIrqHandler[irq] == 0);
	keIrqHandler[irq] = (FnIrqHandler) pHandler;
}

void keSetIDTEntry(u8 vec, void* base, u16 selector, u8 flag) {
	extern IDT_ENTRY IDT[256];
    IDT[vec].offset_1 = (u32)base & 0xffff;
    IDT[vec].offset_2 = ((u32)base >> 16) & 0xffff;
    IDT[vec].selector = selector;
    IDT[vec].type_attr = flag;
	IDT[vec].zero = 0;
}
  
void keSetIDT() {
	IDTR volatile ir;
	
	memset(keIrqHandler, 0, sizeof(keIrqHandler));
	keSetIDTEntry(0, (void*)keExHandler0, KERNEL_CS, 0x8e);
	keSetIDTEntry(1, (void*)keExHandler1, KERNEL_CS, 0x8e);
	keSetIDTEntry(2, (void*)keExHandler2, KERNEL_CS, 0x8e);
	keSetIDTEntry(3, (void*)keExHandler3, KERNEL_CS, 0x8e);
	keSetIDTEntry(4, (void*)keExHandler4, KERNEL_CS, 0x8e);
	keSetIDTEntry(5, (void*)keExHandler5, KERNEL_CS, 0x8e);
	keSetIDTEntry(6, (void*)keExHandler6, KERNEL_CS, 0x8e);
	keSetIDTEntry(7, (void*)keExHandler7, KERNEL_CS, 0x8e);
	keSetIDTEntry(8, (void*)keExHandler8, KERNEL_CS, 0x8e);
	keSetIDTEntry(9, (void*)keExHandler9, KERNEL_CS, 0x8e);
	keSetIDTEntry(10, (void*)keExHandler10, KERNEL_CS, 0x8e);
	keSetIDTEntry(11, (void*)keExHandler11, KERNEL_CS, 0x8e);
	keSetIDTEntry(12, (void*)keExHandler12, KERNEL_CS, 0x8e);
	keSetIDTEntry(13, (void*)keExHandler13, KERNEL_CS, 0x8e);
	keSetIDTEntry(14, (void*)keExHandler14, KERNEL_CS, 0x8e);
	keSetIDTEntry(16, (void*)keExHandler16, KERNEL_CS, 0x8e);
	
	keSetIDTEntry(IRQ_BASE_VECTOR + 0, (void*)keIrqHandler0, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 1, (void*)keIrqHandler1, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 2, (void*)keIrqHandler2, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 3, (void*)keIrqHandler3, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 4, (void*)keIrqHandler4, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 5, (void*)keIrqHandler5, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 6, (void*)keIrqHandler6, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 7, (void*)keIrqHandler7, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 8, (void*)keIrqHandler8, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 9, (void*)keIrqHandler9, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 10, (void*)keIrqHandler10, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 11, (void*)keIrqHandler11, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 12, (void*)keIrqHandler12, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 13, (void*)keIrqHandler13, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 14, (void*)keIrqHandler14, KERNEL_CS, 0x8e);
	keSetIDTEntry(IRQ_BASE_VECTOR + 15, (void*)keIrqHandler15, KERNEL_CS, 0x8e);
	
	
	ir.limit = 256 * sizeof(IDT_ENTRY) - 1;
	ir.base = (u32)IDT;

	// load idt
	__asm__ __volatile__ ("lidt %0": "=m" (ir));
}

