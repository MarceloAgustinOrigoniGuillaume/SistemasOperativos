/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_ENV_H
#define JOS_KERN_ENV_H

#define MAX_PRIORITY 1
#define MIN_PRIORITY 10
#ifdef SCHED_PRIORITIES
#define BOOST_TIMESLICE 150

#define RUNS_UNTIL_LOWER 5
#define LOWER_ON_INTERRUPT 3

#define PRIO_IND(env) env->env_priority - MAX_PRIORITY

#endif
#define GET_ID(env) env ? env->env_id : -1

#include <inc/env.h>
#include <kern/cpu.h>

extern unsigned int last_boost_yield;
extern unsigned int count_sched_yields;
extern unsigned int count_total_runs;
extern unsigned int total_envs_finished;
extern unsigned int total_turnaround;
extern unsigned int max_turnaround;
extern unsigned int total_response_time;
extern unsigned int max_response_time;
extern int tot_slice_switches;

#ifdef SCHED_PRIORITIES
struct PriorityInfo {
	struct Env *first;
	struct Env *last;
};

extern struct PriorityInfo priorities[MIN_PRIORITY];  // All priorities

// Funciones para prioridad!!
void lower_priority_of(struct Env *env, int priority);
void update_priority_fork(struct Env *forked, struct Env *original);

void snapshot();

struct Env *search_runnable_on_p(int indPriority, struct Env **prev);
struct Env *search_runnable_on(struct Env *first, struct Env **prev);
struct Env *search_prev_on_p(struct Env *first, struct Env *target);
struct Env *search_prev_for_p(struct Env *target);

void check_boost_all();

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
// The following functions do not return
// En priorities en teoria deberia llamarse a la que le da el prev. Pero se
// puede delegar llamando a la de un parametro
#ifdef SCHED_PRIORITIES
void env_run(struct Env *e) __attribute__((noreturn));
void env_run_p(struct Env *e, struct Env *prev_on_priority)
        __attribute__((noreturn));
#endif

#ifdef SCHED_ROUND_ROBIN
void env_run(struct Env *e) __attribute__((noreturn));
#endif

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
