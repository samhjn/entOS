[section .text]

global _keSetGDT
_keSetGDT:
	push ebp
	mov ebp, esp
	mov ecx, [ebp+8]
	lgdt [ecx]
	jmp 0x08:jmpto 
jmpto:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	leave
	retn
	
%macro  CRXFunc 1
global _keGetCR%1, _keSetCR%1
_keGetCR%1:
	mov eax, cr%1
	retn
_keSetCR%1:
	push ebp
	mov ebp,esp
	mov eax, [ebp+8]
	mov cr%1, eax
	leave
	retn
%endmacro

CRXFunc 0
CRXFunc 2
CRXFunc 3
