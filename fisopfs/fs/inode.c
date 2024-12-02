#include "./inode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
setNewName(struct Inode *inode, const char *name)
{
	int count = name ? strlen(name) : 0;
	if (count <= 0) {
		perror("Invalid name for new inode\n");
		return 1;
	}

	char *tmpname = (char *) malloc(sizeof(char) * count + 1);

	if (!tmpname) {
		perror("Failed to allocate memory for inode name\n");
		return 1;
	}

	memcpy(tmpname, name, count);
	*(tmpname + count) = 0;
	inode->name = tmpname;
	return 0;
}
