# sched

### Parte 1: Context switch

![alt_text](cs_informe_imagenes/context_switch_code.png "code")

#### Estado inicial del stack

Antes de mover el trapframe al %esp.

![alt_text](cs_informe_imagenes/context_switch_start_stack.png "stack inicial")

Luego de ejecutar `-> add $0x4, %esp` para ignorar el return address.

![alt_text](cs_informe_imagenes/context_switch_stack_after_first_add.png "first instruction")

Ejecuto la siguiente instrucción  `-> pop %esp` para mover al stack pointer el trapframe y poder restaurar los registros.

![alt_text](cs_informe_imagenes/context_switch_registers_after_pop_esp.png "registers_pop_esp")

![alt_text](cs_informe_imagenes/context_switch_stack_after_pop_esp.png "pop_esp")

`-> popal // Restauramos los registros de uso general`

![alt_text](cs_informe_imagenes/context_switch_registers_after_popal.png "registers_popal")

![alt_text](cs_informe_imagenes/context_switch_stack_after_popal.png "stack_popal")

`-> pop %es // Restauramos es`

![alt_text](cs_informe_imagenes/context_switch_stack_after_pop_es.png "pop_es")

` -> pop %ds // Restauramos ds`

![alt_text](cs_informe_imagenes/context_switch_stack_after_pop_ds.png "pop_ds")

`-> add $0x08, %esp // Desplazamos el stack pointer al inicio de tf_eip`

![alt_text](cs_informe_imagenes/context_switch_stack_after_final_add.png "last instruction")

`iret // Volvemos con iret modificando los registros eip, cs y esp tomando los valores restantes del stack `

![alt_text](cs_informe_imagenes/context_switch_registers_after_iret.png "after_iret")
