[section .text]

global _keSwapTaskInternal
_keSwapTaskInternal:
	pushad
	pushfd


	mov eax, [esp + 8 * 4 + 4 + 4 + 4]	;swapToTask
	mov ecx, [esp + 8 * 4 + 4 + 4]		;currentTask
	
	mov [ecx] , esp
	mov esp, [eax]
	popfd
	popad
	retn
	

