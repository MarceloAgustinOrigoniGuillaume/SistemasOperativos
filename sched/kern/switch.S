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

/*
        |    ...    |  mas cosas de antes.
        |  env_tf*  |   
        |  ret addr |   <- esp 
*/
.globl context_switch;
context_switch:
        
	add $0x4, %esp // ignoramos el ret addr
        pop %esp // el stack apuntaria a envtf*, pone el valor , el puntero como esp.
        
	popal         // restauramos los registros
	
	pop %es         // restauramos es
	pop %ds         // restauramos ds
	add $0x8, %esp // Desplazamos el stack pointer al inicio de tf_eip

	iret

spin:
	jmp spin
