// hello, priority
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    int maxLoops = 100000000;
    int priority;
    if (sys_get_priority(thisenv->env_id, &priority)) {
        cprintf("If this prints we are cooked\n");
    }
    cprintf("Hello, my initial priority is %d\n", priority);
    int filler;
    for (int i = 0; i < maxLoops; i++) { filler++; }

    if (sys_get_priority(thisenv->env_id, &priority)) {
        cprintf("If this prints we are cooked\n");
    }
    cprintf("Hello, after so many loops my priority is now %d\n", priority);
}