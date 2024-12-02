#include "./inodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cant_inodes = 0;

struct Inode *root_inode = NULL;
static int new_inodo_id = 1;

static struct Inode *
setBaseInode(int id)
{
	struct Inode *inode = &inodes[id];

	inode->id = id;
	return inode;
}

void
serializeInodeData(struct SerialFD *fd_out, struct Inode *inode)
{
	printf("SERIALIZE inode: %d %s first: %d\n",
	       inode->id,
	       inode->name,
	       inode->first_block);
	writeInt(fd_out, inode->id);
	writeStr(fd_out, inode->name);
	writeInt(fd_out, inode->type);
	writeInt(fd_out, inode->size_bytes);
	writeInt(fd_out, inode->blocks);
	writeInt(fd_out, inode->first_block);
	writeInt(fd_out, inode->permissions);
	writeInt(fd_out, inode->created);
	writeInt(fd_out, inode->modified);
	writeInt(fd_out, inode->last_access);
}

void
deserializeInodeData(struct SerialFD *fd_in, struct Inode *inode)
{
	printf("DESERIALIZE inode id: %d\n", inode->id);
	readStr(fd_in, &inode->name);
	int temp;
	readInt(fd_in, &temp);
	inode->type = temp;

	readInt(fd_in, &inode->size_bytes);
	readInt(fd_in, &inode->blocks);
	readInt(fd_in, &inode->first_block);
	readInt(fd_in, &inode->permissions);
	readInt(fd_in, &temp);
	inode->created = temp;

	readInt(fd_in, &temp);
	inode->modified = temp;

	readInt(fd_in, &temp);
	inode->last_access = temp;
	printf("INODE %d DESERIAL GOT NAME %s and first block %d\n",
	       inode->id,
	       inode->name,
	       inode->first_block);
}


void
serializeInodes(struct SerialFD *fd_out)
{  // Persona 4/1?
	printf("SERIALIZE inodes fd: %d \n", fd_out->fd);

	// First root...
	serializeInodeData(fd_out, root_inode);


	printf("CANT INODES TO WRITE %d\n", cant_inodes);
	writeInt(fd_out, cant_inodes);

	int left = cant_inodes;
	struct Inode *next_free = free_inode;
	int i = 1;  // Skip root
	while (next_free && left > 0) {
		if (i == next_free->id) {  // skip free ones
			next_free = next_free->next_free;
			i++;
			continue;
		}
		serializeInodeData(fd_out, &inodes[i]);
		i++;
		left--;
	}

	while (left > 0) {
		serializeInodeData(fd_out, &inodes[i]);
		i++;
		left--;
	}
}

void
deserializeInodes(struct SerialFD *fd_in)
{  // Persona 4/1?
	printf("DESERIALIZE inodes fd: %d \n", fd_in->fd);
	// Deserial root
	root_inode = setBaseInode(0);
	readInt(fd_in, &root_inode->id);

	deserializeInodeData(fd_in, root_inode);
	printf("---> ROOT INODE %s id: %d\n", root_inode->name, root_inode->id);

	int res = readInt(fd_in, &cant_inodes);
	printf("CANT INODES READED %d\n", cant_inodes);

	int last_id = 0;
	free_inode = setBaseInode(1);
	struct Inode *curr_free = free_inode;

	for (int i = 0; i < cant_inodes; i++) {
		int id = 0;
		res = readInt(fd_in, &id);
		if (res == -1) {
			printf("FAILED READ OF BLOCK ID!");
			return;
		}
		deserializeInodeData(fd_in, setBaseInode(id));

		while (++last_id < id) {  // Add as next free!
			curr_free->next_free = setBaseInode(last_id);
			curr_free = curr_free->next_free;
		}
	}


	new_inodo_id = last_id + 1;
	free_inode = free_inode->next_free;
	if (free_inode == NULL) {
		free_inode = setBaseInode(new_inodo_id++);
	}
	printf("LAST ID WAS %d\n", last_id);
	printf("FREE INODE WAS %d\n", free_inode->id);

	// Pop first free que era un placeholder.
}

void
initInodes()
{
	root_inode = setBaseInode(0);
	free_inode = setBaseInode(1);
	new_inodo_id = 2;

	root_inode->name = "/";
	root_inode->type = I_DIR;
}

void
statOf(struct Inode *inode, struct stat *st)
{  // Persona 4
	printf("STAT OF %s!\n", inode->name);

	if (inode->type == I_DIR) {
		st->st_mode = __S_IFDIR | inode->permissions;
		st->st_nlink = 2;
		st->st_size = 4096;
	} else {
		st->st_mode = __S_IFREG | inode->permissions;
		st->st_nlink = 1;

		st->st_size = inode->size_bytes;
	}

	st->st_uid = inode->id;
	st->st_blocks = inode->blocks;
	st->st_atime = inode->last_access;
	st->st_mtime = inode->modified;
	st->st_ctime = inode->created;
}

struct Inode *
getinode(inode_id_t id)
{
	if (id < 0 || id >= new_inodo_id) {
		printf("Invalid inode ID: %d\n", id);
		return NULL;
	}
	return &inodes[id];
}


void
deleteInode(struct Inode *inode)
{
	printf("DEL INODE %d\n", inode->id);

	free(inode->name);

	inode->name = NULL;
	inode->type = 0;

	inode->size_bytes = 0;
	inode->blocks = 0;
	inode->first_block = -1;


	inode->id = -1;
	inode->permissions = 0;
	inode->created = 0;
	inode->modified = 0;
	inode->last_access = 0;
	inode->next_free = free_inode;
	free_inode = inode;
	cant_inodes--;  // Reduzco la cantidad de inodes
}


struct Inode *
createInode(const char *name, enum InodeType type)
{
	struct Inode *inode;
	if (free_inode->next_free == NULL) {
		free_inode->next_free = setBaseInode(new_inodo_id++);
	}

	inode = free_inode;
	printf("FREE INODE WAS %d\n", inode->id);

	if (setNewName(inode, name) != 0) {
		return NULL;
	}

	// Pop del free inode.
	free_inode = free_inode->next_free;

	inode->type = type;
	inode->permissions = (type == I_DIR) ? 0755 : 0644;

	inode->blocks = 0;
	inode->first_block = NOT_DEFINED_BLOCK;
	inode->size_bytes = 0;

	inode->created = time(NULL);
	inode->modified = inode->created;
	inode->last_access = inode->created;
	inode->next_free = NULL;

	cant_inodes++;  // Aumento la cantidad de inodes
	return inode;
}
