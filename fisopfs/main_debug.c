#define FUSE_USE_VERSION 30

#include "fs/serial.h"
#include <stdio.h>
#include <sys/types.h>

#include <string.h>
#include <stdlib.h>
#include "fs/inode.h"


#include "debug/asserts.h"
#include "debug/tests_serial.h"


static void
saveInode(struct SerialFD *writer, struct Inode *inode)
{
	writeInt(writer, inode->id);
	printf("WROTE ID %d\n", inode->id);
	writeStr(writer, inode->name);
	writeInt(writer, 44);  // Perm?
}

static void
loadInode(struct SerialFD *writer, struct Inode *inode)
{
	readInt(writer, &inode->id);
	printf("ID IS %d\n", inode->id);
	readStr(writer, &inode->name);
	printf("NAME IS %s\n", inode->name);
	int prm;
	readInt(writer, &prm);  // Perm?
	printf("PREM %d\n", prm);
}


static int
testInode(const char *filename)
{
	printf("TEST INODE?!---------- %s\n", filename);
	int err = 0;
	struct SerialFD writer = openWriter(filename, &err);

	struct Inode wrInode;

	wrInode.name = "INODO 2!";
	wrInode.id = 43;

	saveInode(&writer, &wrInode);
	saveInode(&writer, &wrInode);

	wrInode.name = "INODO2321!";
	wrInode.id = 321;
	saveInode(&writer, &wrInode);
	closeWriter(&writer);

	printf("WROTE ALL GOOD?!---------- read: %s\n", filename);
	struct SerialFD reader = openReader(filename, &err);
	struct Inode inodo;

	loadInode(&reader, &inodo);
	loadInode(&reader, &inodo);
	loadInode(&reader, &inodo);

	closeWriter(&reader);

	return 0;
}

int
main(int argc, char *argv[])
{
	/*
	int data = 512+ 254;
	char* buff = "UN ELEMENTO!";
	showBytes((uint8_t*)buff, strlen(buff));
	printf("----------\n");
	buff = (char*) &data;
	showBytes((uint8_t*)buff, sizeof(data));
	*/

	setPath(argv[1]);

	int count = countSerialTests();

	allocTests(count);

	initSerialTests();

	int failed = runTests(0);  // 1 para que sea fail fast.

	printf("----------Failed %d of %d\n", failed, testCount());
	freeTests();

	return failed > 0 ? 1 : 0;
}
