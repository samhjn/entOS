#include "os.h"

#define PIC1		0x20		
#define PIC2		0xA0		
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20
#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */


void kePICInit() {
	u8 a1, a2;
 
	a1 = inportb(PIC1_DATA);                        
	a2 = inportb(PIC2_DATA);
 
	outportb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  
	io_wait();
	outportb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outportb(PIC1_DATA, IRQ_BASE_VECTOR);                 
	io_wait();
	outportb(PIC2_DATA, IRQ_BASE_VECTOR + 8);                 
	io_wait();
	outportb(PIC1_DATA, 4);                      
	io_wait();
	outportb(PIC2_DATA, 2);                       
	io_wait();
 
	outportb(PIC1_DATA, ICW4_8086);
	io_wait();
	outportb(PIC2_DATA, ICW4_8086);
	io_wait();
 
	outportb(PIC1_DATA, a1);   
	outportb(PIC2_DATA, a2);
	
	kePICSetMask(0xffff); 
}

void kePICDoEoi(int irq) {
	if(irq >= 8)
		outportb(PIC2_COMMAND, PIC_EOI);
	outportb(PIC1_COMMAND, PIC_EOI);
}

u32 kePICGetMask() {
	u32 a1,a2;
	
	a1 = inportb(PIC1_DATA);                        
	a2 = inportb(PIC2_DATA);
	return (a2 << 8) + a1;
}

void kePICSetMask(u32 m) {
	u32 a1, a2;
	
	a1 = (m & 0x000000ff);
	a2 = m >> 8;
	outportb(PIC1_DATA, a1);
	outportb(PIC2_DATA, a2);
	//kprintf("kePICSetMask: %08x %08x\n", m, kePICGetMask());
}

void kePICDisableIrq(int irq) {
	kePICSetMask(kePICGetMask() | (1 << irq));
}

void kePICEnableIrq(int irq) {
	kePICSetMask(kePICGetMask() & (~(1 << irq)));
}
