#include "./asserts.h"
#include "./tests_serial.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../fs/serial.h"
#include "../fs/inodes.h"
#include "../fs/blocks.h"
#include "../fs/directories.h"

static char *filename = NULL;  //= "tmp_serial";
void
setPath(char *file)
{
	filename = file;
	printf("SET SERIAL TEST TARGET FILE %s\n", file);
}


static void
setBasicInode(struct Inode *inode, int id, enum InodeType type)
{
	inode->id = id;
	inode->type = type;

	inode->permissions = (inode->type == I_DIR) ? 0755 : 0644;

	inode->blocks = 0;
	inode->first_block = NOT_DEFINED_BLOCK;
	inode->size_bytes = 0;

	inode->created = 204313;
	inode->modified = inode->created;
	inode->last_access = inode->created;
	inode->next_free = NULL;
}

static int
assertInodeEQ(struct Inode *given, struct Inode *expected)
{
	EXPECT_EQ(given->id, expected->id, "Ids no eran el mismo! given: %d");
	EXPECT_EQ_STR(given->name,
	              expected->name,
	              "Names no eran el mismo! given: %s");

	EXPECT_EQ(given->type, expected->type, "Types no eran el mismo! given: %d");
	EXPECT_EQ(given->blocks,
	          expected->blocks,
	          "Blocks count no eran el mismo! given: %d");
	EXPECT_EQ(given->first_block,
	          expected->first_block,
	          "First block no eran el mismo! given: %d");

	EXPECT_EQ(given->size_bytes,
	          expected->size_bytes,
	          "Size bytes no eran el mismo! given: %d");
	EXPECT_EQ(given->permissions,
	          expected->permissions,
	          "Permissions no eran el mismo! given: %d");

	EXPECT_EQ(given->created,
	          expected->created,
	          "Created time no eran el mismo! given: %ld");
	EXPECT_EQ(given->modified,
	          expected->modified,
	          "Modified time no eran el mismo! given: %ld");
	EXPECT_EQ(given->last_access,
	          expected->last_access,
	          "Last access time no eran el mismo! given: %ld");

	return 0;
}


static void
setBasicDir(struct DirData *dir, int id, int size)
{
	dir->id_dir = id;
	dir->size = size;  // 0 children!
	dir->next_free = NULL;

	dir->capacity = 5;

	// Por suerte ya estan en code segment,... no en el heap!
	for (int i = 0; i < dir->capacity; i++) {
		dir->entries_id[i] = NOT_DEFINED_BLOCK;
	}
}

static int
assertDirEQ(struct DirData *given, struct DirData *expected)
{
	EXPECT_EQ(given->id_dir,
	          expected->id_dir,
	          "Ids no eran el mismo para dir! given: %d");
	EXPECT_EQ(given->size,
	          expected->size,
	          "Size no eran el mismo para dir! given: %d");
	EXPECT_EQ(given->capacity,
	          expected->capacity,
	          "Capacity no eran el mismo para dir! given: %d");

	for (int i = 0; i < expected->capacity; i++) {
		EXPECT_EQ(given->entries_id[i],
		          expected->entries_id[i],
		          "Child no era el mismo para dir! given: %d");
	}

	return 0;
}


static void
setBasicBlock(struct Block *block, int id, const char *toWrite)
{
	block->id = id;
	block->size = 0;
	if (toWrite) {
		writeToBlock(block, toWrite, 0, strlen(toWrite));
	}
}

static int
assertBlockEQ(struct Block *given, struct Block *expected)
{
	EXPECT_EQ(given->id,
	          expected->id,
	          "Ids no eran el mismo para file! given: %d");
	EXPECT_EQ(given->size,
	          expected->size,
	          "Size no eran el mismo para file! given: %d");

	for (int i = 0; i < expected->size; i++) {
		if (given->data[i] != expected->data[i]) {
			printf("Content was not the same! at %d given: %c exp: "
			       "%c\n",
			       i,
			       given->data[i],
			       expected->data[i]);
			return 1;
		}
	}

	return 0;
}


int
some_err()
{
	return 3;
}
int
test_simple_fail()
{
	int err;

	EXPECT_EQ(2, 1, "deberia ser 1! %d");

	ASSERT_EQ(err, some_err(), 1, "deberia ser 1 ASSERT! %d");

	return 0;
}

int
test_simple_success()
{
	int err;

	EXPECT_EQ(1, 1, "deberia ser 1! %d");

	ASSERT_EQ(err, some_err(), 3, "deberia ser 3 ASSERT! %d");

	return 0;
}

int
test_simple_inode()
{
	int err = 0;

	struct Inode wrInode;
	setBasicInode(&wrInode, 43, I_DIR);
	wrInode.name = " UN VALOR NAME!";

	CHECK_SUCCESS(err, assertInodeEQ(&wrInode, &wrInode));  // Por las dudas

	struct SerialFD writer = openWriter(filename, &err);

	EXPECT_EQ(err, 0, "Hubo error al abrir el writer err %d");

	serializeInodeData(&writer, &wrInode);
	closeWriter(&writer);


	struct SerialFD reader = openReader(filename, &err);
	EXPECT_EQ(err, 0, "Hubo error al abrir el reader err %d");


	struct Inode rdInode;


	readInt(&reader, &rdInode.id);
	deserializeInodeData(&reader, &rdInode);

	CHECK_SUCCESS(err, assertInodeEQ(&rdInode, &wrInode));

	closeWriter(&reader);


	return 0;
}


int
test_simple_dir()
{
	int err = 0;

	struct DirData wrDir;
	setBasicDir(&wrDir, 2, 2);  // id, size
	wrDir.entries_id[1] = 3;
	wrDir.entries_id[2] = 1;

	CHECK_SUCCESS(err, assertDirEQ(&wrDir, &wrDir));  // Por las dudas


	struct SerialFD writer = openWriter(filename, &err);

	EXPECT_EQ(err, 0, "Hubo error al abrir el writer err %d");

	serializeDirData(&writer, &wrDir);
	closeWriter(&writer);


	struct SerialFD reader = openReader(filename, &err);
	EXPECT_EQ(err, 0, "Hubo error al abrir el reader err %d");


	struct DirData rdDir;


	readInt(&reader, &rdDir.id_dir);
	deserializeDirData(&reader, &rdDir);

	CHECK_SUCCESS(err, assertDirEQ(&rdDir, &wrDir));

	closeWriter(&reader);
	return 0;
}


int
test_simple_file()
{
	int err = 0;

	struct Block wrBlock;
	setBasicBlock(&wrBlock, 2, "UN TEXTO!");  // id, size

	CHECK_SUCCESS(err, assertBlockEQ(&wrBlock, &wrBlock));  // Por las dudas


	struct SerialFD writer = openWriter(filename, &err);

	EXPECT_EQ(err, 0, "Hubo error al abrir el writer err %d");

	serializeBlockData(&writer, &wrBlock);
	closeWriter(&writer);


	struct SerialFD reader = openReader(filename, &err);
	EXPECT_EQ(err, 0, "Hubo error al abrir el reader err %d");


	struct Block rdBlock;


	readInt(&reader, &rdBlock.id);
	deserializeBlockData(&reader, &rdBlock);

	CHECK_SUCCESS(err, assertBlockEQ(&rdBlock, &wrBlock));

	closeWriter(&reader);
	return 0;
}


int
test_simple()
{
	int err = 0;
	struct SerialFD writer = openWriter(filename, &err);

	EXPECT_EQ(err, 0, "Hubo error al abrir el writer err %d");

	ASSERT_EQ(err, writeInt(&writer, 25), 0, "Hubo error al escribir el 25,err %d")
	ASSERT_EQ(err, writeStr(&writer, "UNO"), 0, "Error al escribir UNO,err %d")
	ASSERT_EQ(err, writeInt(&writer, 50), 0, "Hubo error al escribir el 50,err %d")

	closeWriter(&writer);


	struct SerialFD reader = openReader(filename, &err);
	EXPECT_EQ(err, 0, "Hubo error al abrir el reader err %d");

	int out;
	char *outStr = NULL;

	ASSERT_EQ(err, readInt(&reader, &out), 0, "Hubo error al leer numero, err %d")
	EXPECT_EQ(out, 25, "El valor leido para 25 fue %d")


	ASSERT_EQ(err, readStr(&reader, &outStr), 0, "Hubo error al leer string, err %d")
	EXPECT_EQ_STR(outStr, "UNO", "El valor leido de string no fue 'UNO' fue %s")


	ASSERT_EQ(err, readInt(&reader, &out), 0, "Hubo error al leer numero, err %d")
	EXPECT_EQ(out, 50, "El valor leido para 50 fue %d")

	closeWriter(&reader);

	return 0;
}

int
countSerialTests()
{
	return 5;
}

void
initSerialTests()
{
	addTest(&test_simple_success);
	addTest(&test_simple);
	addTest(&test_simple_inode);
	addTest(&test_simple_dir);
	addTest(&test_simple_file);

	// addTest(&test_simple_fail);
}
