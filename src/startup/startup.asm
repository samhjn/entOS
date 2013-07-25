extern data, bss, code, _end, _keInit
global start, mboot
[section .code]
align 4
%define MULTIBOOT_PAGE_ALIGN	 (1 << 0)
%define MULTIBOOT_MEMORY_INFO	 (1 << 1)
%define MULTIBOOT_VIDEO			 (1 << 2)
%define MULTIBOOT_AOUT_KLUDGE	 (1 << 16)
%define MULTIBOOT_HEADER_MAGIC	 0x1badb002
%define MULTIBOOT_HEADER_FLAGS	 MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
%define MULTIBOOT_CHECKSUM		 -((MULTIBOOT_HEADER_MAGIC) + (MULTIBOOT_HEADER_FLAGS))
%define STACK_SIZE				4096

start:
	jmp main
	
align 4	
mboot:	
	dd MULTIBOOT_HEADER_MAGIC 
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM
	dd mboot
	dd code
	dd bss
	dd _end
	dd start
main: 
	cli
	mov esp, stack_top
	mov [_magic], eax
	mov [_mbd], ebx
	call _keInit
	cli
	hlt    
	
[section .data]
global _magic, _mbd, _GDT, _IDT
align 4096
stack_start: 
	times STACK_SIZE db 0
stack_top:
	dd 0
_magic:
	dd 0
_mbd:
	dd 0
align 4096
_GDT:
	times 256 dq 0
align 4096
_IDT:
	times 256 dq 0
	