/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

/*
 	mov rax, 0x4 // valor inmediato a registro
	mov [rdx], -147 // valor inmediato a memoria
	mov rdx, rax // registro a registro
	mov [rdx], rax //registro a memoria
	mov rdx, [rax] //memoria a registro
*/

.globl context_switch;
context_switch:

    pop eax      // Popeamos el puntero del trapframe en eax
	//mov ebx, rsp // backupeamos el stackpointer
	mov rsp, eax // sobreescribimos el stack pointer con el trapframe
	//push ebx     // pusheamos el stackpointer original al stack
	popal        // restauramos los registros
	//pop rsp      // restauramos el stack pointer

	add rsp, 16 // Desplazamos el stack pointer al inicio de tf_eip



	iret

spin:
	jmp spin
