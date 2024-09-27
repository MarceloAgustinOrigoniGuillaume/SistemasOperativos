#include "exec.h"
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define REDIR_TO_OUT "&1"
// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int idx = block_contains(eargv[i], EQUAL_SYMBOL);
		if (idx < 0) {
			continue;
		}

		char *key = malloc(ARGSIZE);
		char *value = malloc(ARGSIZE);

		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);

		int result =
		        setenv(key, value, 1);  // no quiero que sobreescriba.

		free(key);
		free(value);

		if (result != 0) {
			perror("Error en asignación de variable de entorno.");
			_exit(EXIT_FAILURE);
		}
	}
}

static void
unset_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		char *key = malloc(ARGSIZE);

		get_environ_key(eargv[i], key);

		int result = unsetenv(key);

		free(key);

		if (result != 0) {
			perror("Error en la eliminación de variable de "
			       "entorno.");
			_exit(EXIT_FAILURE);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	if (strlen(file) > 0) {
		if (flags == O_RDONLY) {
			return open(file, flags);
		}

		if (flags == O_WRONLY) {
			return open(file,
			            flags | O_CREAT | O_TRUNC,
			            S_IRUSR | S_IWUSR);
		}

		return open(file, flags, S_IRUSR | S_IWUSR);
	}

	return -2;
}


void
simple_exec(struct execcmd *e)
{
	set_environ_vars(e->eargv, e->eargc);
	execvp(e->argv[0], e->argv);
	unset_environ_vars(e->eargv, e->eargc);

	printf("Commands are not yet implemented\n");
	_exit(EXIT_FAILURE);
}


int
waitAndGetRet(int pid)
{
	int status;
	waitpid(pid, &status, 0);
	if (WIFSIGNALED(status)) {
		return 1;
	} else if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	}


	return 0;
}

int
divide_pipe(struct pipecmd *p)
{
	int fdPipe[2];
	if (pipe(fdPipe) < 0) {
		fprintf(stderr, "Pipe creation for divide failed\n");
		_exit(EXIT_FAILURE);
	}

	int pidleft = fork();

	if (pidleft < 0) {
		fprintf(stderr, "Fork for left process failed\n");
		_exit(EXIT_FAILURE);
	}

	if (pidleft == 0) {
		close(fdPipe[0]);
		dup2(fdPipe[1], 1);
		close(fdPipe[1]);

		simple_exec(p->leftcmd);
	}

	close(fdPipe[1]);

	int pidright = fork();

	if (pidright < 0) {
		fprintf(stderr, "Fork for right process failed\n");
		_exit(EXIT_FAILURE);
	}

	if (pidright == 0) {
		dup2(fdPipe[0], 0);
		close(fdPipe[0]);

		exec_cmd(p->rightcmd);
	}
	close(fdPipe[0]);

	waitAndGetRet(pidleft);

	waitAndGetRet(pidright);
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases

	struct execcmd *r;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		r = (struct execcmd *) cmd;
		simple_exec(r);
		break;

	case BACK: {
		// runs a command in background
		//
		// Your code here
		struct backcmd *b = cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		struct execcmd *r = cmd;
		int fdFile = open_redir_fd(r->in_file, O_RDONLY);
		if (fdFile >= 0) {
			dup2(fdFile, 0);
			close(fdFile);
		} else if (fdFile == -1) {
			_exit(EXIT_FAILURE);
		}

		fdFile = open_redir_fd(r->out_file, O_WRONLY);
		if (fdFile > 0) {
			dup2(fdFile, 1);
			close(fdFile);
		} else if (fdFile == -1) {
			_exit(EXIT_FAILURE);
		}

		if (strncmp(r->err_file, REDIR_TO_OUT, 2) == 0) {
			dup2(1,
			     2);  // En vez de un archivo, lo que sea que apunte el stdout =1
		} else {
			fdFile = open_redir_fd(r->err_file, O_WRONLY);
			if (fdFile > 0) {
				dup2(fdFile, 2);
				close(fdFile);
			} else if (fdFile == -1) {
				_exit(EXIT_FAILURE);
			}
		}
		simple_exec(r);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		int ret = divide_pipe((struct pipecmd *) cmd);

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);
		_exit(ret);
		break;
	}
	}
}
