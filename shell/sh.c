#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

#include "sys/wait.h"
#include "signal.h"
#include "stdio.h"
#include "errno.h"


char prompt[PRMTLEN] = { 0 };

static void
handle_end(int num)
{
	int status;
	// Wait for any process end with group pid == main_pid.
	pid_t pid = waitpid(0, &status, WNOHANG);

	if (pid < 0) {
		return;  // Not a background process..
	}

	printf_debug("==> terminado: %d\n", pid);
}

static void
sethandler()
{
	stack_t stack;
	stack.ss_sp = malloc(SIGSTKSZ);
	if (stack.ss_sp == NULL) {
		printf_debug("stack malloc failed");
		_exit(EXIT_FAILURE);
	}

	stack.ss_size = SIGSTKSZ;
	stack.ss_flags = 0;
	if (sigaltstack(&stack, NULL) == -1) {
		printf("ENTRO SET HAND %d\n", errno);
		printf_debug("sigaltstack failed");
		_exit(EXIT_FAILURE);
	}

	struct sigaction sa;
	sa.sa_handler = &handle_end;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_ONSTACK;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		printf_debug("sigaction failed");
		_exit(EXIT_FAILURE);
	}
}


// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	sethandler();
	init_shell();

	run_shell();

	return 0;
}
