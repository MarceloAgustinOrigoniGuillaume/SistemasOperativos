/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_ENV_H
#define JOS_KERN_ENV_H

#define MAX_PRIORITY 1
#define MIN_PRIORITY 4
#define BOOST_TIMESLICE 100
#include <inc/env.h>
#include <kern/cpu.h>


extern int last_boost_yield;
extern int count_sched_yields;
extern unsigned int total_envs_finished;
extern unsigned int total_turnaround;
extern unsigned int total_response_time;
extern int tot_slice_switches;
//extern struct EnvFinished * finished_envs;           // All environments

#ifdef SCHED_PRIORITIES
struct PriorityInfo{
      struct Env* first;
      struct Env* last;
};

extern struct PriorityInfo priorities[MIN_PRIORITY]; // All priorities

// Funciones para prioridad!!
void lower_priority_env(struct Env *env, struct Env *prev);

void remove_from_priority(struct Env *env, struct Env *prev);
void add_to_priority(struct Env *env, int ind);
void snapshot();

struct Env* search_runnable_on_p(int indPriority, struct Env ** prev);
struct Env* search_runnable_on(struct Env * first, struct Env ** prev);
struct Env* search_prev_on_p(struct Env * first, struct Env * target);
struct Env* search_prev_for_p(struct Env * target);

void boost_all();

//struct Env* update_priority(struct Env * target, int new_priority);
#endif


extern struct Env *envs;           // All environments
#define curenv (thiscpu->cpu_env)  // Current environment
extern struct Segdesc gdt[];

void env_init(void);
void env_init_percpu(void);
int env_alloc(struct Env **e, envid_t parent_id);
void env_free(struct Env *e);
void env_load_pgdir(struct Env *e);
void env_create(uint8_t *binary, enum EnvType type);
void env_destroy(struct Env *e);  // Does not return if e == curenv

int envid2env(envid_t envid, struct Env **env_store, bool checkperm);
// The following two functions do not return
void env_run(struct Env *e) __attribute__((noreturn));
void context_switch(struct Trapframe *tf) __attribute__((noreturn));




// Without this extra macro, we couldn't pass macros like TEST to
// ENV_CREATE because of the C pre-processor's argument prescan rule.
#define ENV_PASTE3(x, y, z) x##y##z

#define ENV_CREATE(x, type)                                                    \
	do {                                                                   \
		extern uint8_t ENV_PASTE3(_binary_obj_, x, _start)[];          \
		env_create(ENV_PASTE3(_binary_obj_, x, _start), type);         \
	} while (0)

#endif  // !JOS_KERN_ENV_H
