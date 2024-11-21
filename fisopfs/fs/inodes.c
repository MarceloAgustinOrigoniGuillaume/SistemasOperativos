#include "./inodes.h"
#include <stdio.h>
#include <string.h>

struct Inode root_inode = {"/", I_DIR, NULL};

void statOf(struct Inode* inode, struct stat *st){ // Persona 4
    if (strcmp(inode->name, root_inode.name) == 0) {
	    st->st_uid = 1717;
	    st->st_mode = __S_IFDIR | 0755;
	    st->st_nlink = 2;
    } else if (strcmp(inode->name, "somefile") == 0) {
	    st->st_uid = 1818;
	    st->st_mode = __S_IFREG | 0644;
	    st->st_size = 2048;
	    st->st_nlink = 1;
    }
}
struct Inode* getinode(inode_id_t id){ // Persona 4
    return &inodes[id];
}
