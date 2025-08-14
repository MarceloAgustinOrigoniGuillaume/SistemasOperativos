// hello, priority
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    int priority;
    sys_get_priority(thisenv->env_id, &priority);

	cprintf("hello, world\n");
	cprintf("My priority is %d\n", priority);

    cprintf("lowering my priority...\n");
    sys_lower_priority(thisenv->env_id, 4);

    sys_get_priority(thisenv->env_id, &priority);
    cprintf("My priority is now %d\n", priority);
}
