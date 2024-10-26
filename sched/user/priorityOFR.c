// hello, priority
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    int priority;
    int retValue;
    sys_get_priority(thisenv->env_id, &priority);

	cprintf("hello, world\n");
	cprintf("My priority is %d\n", priority);

    cprintf("lowering my priority to an out of range value...\n");
    retValue = sys_lower_priority(thisenv->env_id, 20);
    if (retValue < 0) {
        cprintf("ERROR: Priority out of range\n");
    } else {
        cprintf("This shouldn't happen ever\n");
    }
}
