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

    cprintf("lowering my priority...\n");
    retValue = sys_lower_priority(thisenv->env_id, 4);
    cprintf("The correct retValue is %d\n", retValue);

    sys_get_priority(thisenv->env_id, &priority);
    cprintf("My priority is now %d\n", priority);

    cprintf("now adding priority, this should not modify anything...\n");
    retValue = sys_lower_priority(thisenv->env_id, 1);

    if (retValue < 0) {
        cprintf("You can't up your priority!\n");
    } else {
        cprintf("This shouldn't happen ever\n");
    }
}
