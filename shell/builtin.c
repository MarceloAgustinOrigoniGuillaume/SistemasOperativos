#include "builtin.h"
#include <string.h>

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return (strncmp(cmd, "exit", 4) == 0) ? 1 : 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strncmp(cmd, "cd", 2) != 0)
		return 0;

	char *path = strtok(cmd, " ");
	path = strtok(NULL, " ");

	if (path == NULL) {
		char *home = getenv("HOME");
		if (chdir(home) < 0) {
			perror("ERROR: could not change to home directory");
			return 0;
		}
		snprintf(prompt, sizeof prompt, "(%s)", home);
	} else {
		if (chdir(path) < 0) {
			perror("ERROR: Cannot change directory");
			return 0;
		}
		snprintf(prompt, sizeof prompt, "(%s)", path);
	}

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strncmp(cmd, "pwd", 3) != 0)
		return 0;
	char *cwd = getcwd(NULL, 0);
	if (cwd == NULL) {
		perror("ERROR: could not get current directory");
		return 0;
	}
	printf("%s\n", cwd);
	free(cwd);
	return 1;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
