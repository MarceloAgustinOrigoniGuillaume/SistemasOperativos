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
	// Your code here
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
		// printf("SE TIENE QUE DIRECCIONAR AL ARCHIVO %s\n", file);
		return open(file,
		            flags | O_CREAT,
		            S_IRUSR | S_IWUSR);  //, O_CREAT, S_IWUSR, S_IRUSR);
	}

	return -1;
}

void
simple_exec(struct execcmd *e)
{
	execvp(e->argv[0], e->argv);
	printf("Commands are not yet implemented\n");
	_exit(-1);
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

void
divide_pipe(struct pipecmd *p)
{
	int fdPipe[2];
	if (pipe(fdPipe) < 0) {
		fprintf(stderr, "Pipe creation for divide failed\n");
		_exit(-1);
	}

	int pidleft = fork();


	if (pidleft < 0) {
		fprintf(stderr, "Fork for left process failed\n");
		_exit(-1);
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
		_exit(-1);
	}


	if (pidright == 0) {
		// if(p->rightcmd->type != PIPE){
		//	struct execcmd *temp = p->rightcmd;
		// printf("DEBERIA EJECUTAR PIPE?\n");
		// printf("DERECHA CON %s %s",temp->argv[0],temp->argv[1]);
		//}


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

	struct backcmd *b;
	struct execcmd *r;


	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		simple_exec((struct execcmd *) cmd);
		break;

	case BACK: {
		// runs a command in background
		//
		// Your code here
		printf("Background process are not yet implemented\n");
		_exit(-1);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		struct execcmd *r = cmd;
		int fdFile = open_redir_fd(r->in_file, O_RDONLY);
		if (fdFile >= 0) {
			// printf("DEBERIA REDIR INPUT A %s \n",&r->in_file[0]);
			dup2(fdFile, 0);
			close(fdFile);
		}

		fdFile = open_redir_fd(r->out_file, O_WRONLY);
		// printf("DEBERIA REDIR OUTPUT %d\n",fdFile);
		if (fdFile > 0) {
			dup2(fdFile, 1);
			close(fdFile);
		}

		if (strncmp(r->err_file, REDIR_TO_OUT, 2) == 0) {
			dup2(1,
			     2);  // En vez de un archivo, lo que sea que apunte el stdout =1
		} else {
			fdFile = open_redir_fd(r->err_file, O_WRONLY);
			// fprintf(stderr,"DEBERIA REDIR ERR %d\n",fdFile);
			if (fdFile > 0) {
				// printf("DEBERIA REDIR INPUT A %s ",&r->in_file[0]);
				dup2(fdFile, 2);
				close(fdFile);
			}
		}
		// fprintf(stderr,"DEBERIA REDIR ERR %d\n",stderr);

		// printf("SALIÓ TODO BIEN CON LOS ARCHIVOS Y VOY A EJECUTAR: \n");
		simple_exec(r);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		divide_pipe((struct pipecmd *) cmd);
		// printf("Despues del divide pipe\n");

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);
		_exit(-1);
		break;
	}
	}
}
