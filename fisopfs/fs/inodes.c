#include "./inodes.h"
#include <stdio.h>

void statOf(struct Inode* inode, struct stat *st){ // Persona 4
    st->st_uid = 1717; //id?
    printf("Get Stats of %s\n", inode->name);
}
struct Inode* getinode(inode_id_t id){ // Persona 4
    return &inodes[id];
}
