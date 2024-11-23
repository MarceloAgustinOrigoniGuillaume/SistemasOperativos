#include "./inodes.h"
#include <stdio.h>
#include <string.h>

struct Inode* root_inode= NULL;

void statOf(struct Inode* inode, struct stat *st){ // Persona 4
    printf("STAT OF %s!\n",inode->name);

    if (strcmp(inode->name, "/") == 0) {
	    st->st_uid = inode->id;
	    st->st_mode = __S_IFDIR | 0755;
	    st->st_nlink = 2;
    } else if (strcmp(inode->name, "somefile") == 0) {
	    st->st_uid = 2;
	    st->st_mode = __S_IFREG | 0664;
	    st->st_size = 13;
	    st->st_nlink = 1;
    }
}
struct Inode* getinode(inode_id_t id){ // Persona 4
    return &inodes[id];
}

void deleteInode(struct Inode* inode){ // Persona 4
     printf("DEL INODE %d\n", inode->id);
}

static int curr = 1;
struct Inode* createInode(char* name, enum InodeType type){ // Persona 4
     inodes[curr].name = name;
     inodes[curr].type = type;
     
     return &inodes[curr++];
}
