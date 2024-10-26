#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
//#include <kern/sched.h>
#include <kern/pmap.h>
#include <kern/monitor.h>



static unsigned int count_sched_idle = 0;

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin
	int currind = curenv == NULL ?  -1 : ENVX(curenv->env_id);
	int ind = currind+1;
	while(ind < NENV && envs[ind].env_status != ENV_RUNNABLE){ // iteramos el arreglo hasta el final.
                ind++;
        }
         
        if(ind < NENV ){ // Se encontro
            //cprintf("----- Found process ind: %d!\n",ind);
            env_run(&envs[ind]);
        }
         
        ind = 0;
	while(ind < currind && envs[ind].env_status != ENV_RUNNABLE){ // iteramos el arreglo circularmente.
	    ind++;
        }
         
         
        if (ind < currind){
            //cprintf("----- Found process ind: %d!\n",ind);
            env_run(&envs[ind]);
        }        
        // Found nothing , if not runnning then reset to null
        if(curenv && envs[ind].env_status != ENV_RUNNING){
            //cprintf("----- Not found process y no estaba running para continuar!\n");
            curenv = NULL;
        //} else{
        //     cprintf("----- Not found process keep curr %d!\n",curenv);
        }
        
#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	
	// Search_runnable y
	// Lower priority estan en env.c !
	
	struct Env* prev;
	
	lock_kernel(); // Nadie querria cambien las prioridades en el medio.
	// O que varios procesadores eligan el mismo.
	
	struct Env* next= search_runnable_on_p(0, &prev);
	unlock_kernel();
	
	int nextPriority= 1;
	while(next == NULL && nextPriority < MIN_PRIORITY){
	     next= search_runnable_on_p(nextPriority, &prev);
	     nextPriority++;
	}
		
	if (next != NULL) {
	        if(next->env_runs % 2 == 0){ // Cada 2 env runs..
	             lower_priority_env(next, prev);
	        }
	        unlock_kernel();
		env_run(next);
	}
	
	unlock_kernel();
	
	// No se encontro un next. Fijate si podes seguir con este.. sino reset..
        if(curenv && curenv->env_status != ENV_RUNNING){
            curenv = NULL;
        }
#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;
	cprintf("Fell on sched_halt\n");
        count_sched_idle++;
	// cprintf("In theory, this should show only at the end when theres no more env to run\n");
	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		cprintf("========== ALGUNA ESTADISTICA\n");
		cprintf("========== cantidad de llamados al sched %d\n", count_sched_yields);
		cprintf("========== cantidad timeslices idle %d\n", count_sched_idle);
		cprintf("========== cantidad procesos terminados %d\n", total_envs_finished);
		if(total_envs_finished > 0){
		      cprintf("========== cantidad turnaround promedio %d\n", total_turnaround/total_envs_finished);
		      cprintf("========== cantidad promedio en empezar un env %d\n", total_response_time/total_envs_finished);
		}
		
	        int ind =0;
	        for (ind; ind < NENV; ind++) {
	                if(envs[ind].env_runs == 0){
	                    continue;
	                }
	                
	                cprintf("========== turnaround: %d id: %d runs:%d\n",envs[ind].start, 
	                                                             envs[ind].env_id, envs[ind].env_runs); 
	        }

		
		
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
