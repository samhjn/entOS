[section .text]

extern _keDispatchException, _keDispatchIrq
global UnexpectedHandler

%macro  ExHandlerWithErrno 1
global _keExHandler%1
_keExHandler%1:
	push %1
	jmp  Exception
%endmacro

%macro  ExHandler 1
global _keExHandler%1
_keExHandler%1:
	push 0
	push %1
	jmp Exception
%endmacro

%macro  IrqHandler 1
global _keIrqHandler%1
_keIrqHandler%1:
	push %1
	jmp Irq
%endmacro

ExHandler 0
ExHandler 1
ExHandler 2
ExHandler 3
ExHandler 4
ExHandler 5
ExHandler 6
ExHandler 7
ExHandlerWithErrno 8
ExHandler 9
ExHandlerWithErrno 10
ExHandlerWithErrno 11
ExHandlerWithErrno 12
ExHandlerWithErrno 13
ExHandlerWithErrno 14
ExHandler 16

IrqHandler 0
IrqHandler 1
IrqHandler 2
IrqHandler 3
IrqHandler 4
IrqHandler 5
IrqHandler 6
IrqHandler 7
IrqHandler 8
IrqHandler 9
IrqHandler 10
IrqHandler 11
IrqHandler 12
IrqHandler 13 
IrqHandler 14
IrqHandler 15

Exception:
	cli

	pushad
	call _keDispatchException
	popad
	add esp, 8
	;sti 
	iretd
Irq:
	cli
	pushad
	call _keDispatchIrq
	popad
	add esp, 4
	;sti
	iretd
UnexpectedHandler:
	cli 
	push 0
	push 0xffffffff
	pushad
	call _keDispatchException
	popad
	iretd
	
	
