#include "./inodes.h"
#include <stdio.h>
#include <string.h>

struct Inode* root_inode= NULL;

void statOf(struct Inode* inode, struct stat *st, int uid){ // Persona 4
    printf("STAT OF %s!\n",inode->name);

    // if (strcmp(inode->name, "/") == 0) {
	//     st->st_uid = inode->id;
	//     st->st_mode = __S_IFDIR | 0755;
	//     st->st_nlink = 2;
    // } else if (strcmp(inode->name, "somefile") == 0) {
	//     st->st_uid = 2;
	//     st->st_mode = __S_IFREG | 0664;
	//     st->st_size = inode->data->size;
	//     st->st_nlink = 1;
    // }

    
    if (inode->type == I_DIR){
        st->st_mode = __S_IFDIR | inode->permissions;
        st->st_nlink = 2;
        st->st_size = 4096;
    } else {
        st->st_mode = __S_IFREG | inode->permissions;
        st->st_nlink = 1;
        st->st_size = inode->data ? inode->data->size : 0;
    }

    st->st_uid = uid;
    st->st_blocks = inode->blocks;
    st->st_atime = inode->last_access; 
    st->st_mtime = inode->modified; 
    st->st_ctime = inode->created;
}

struct Inode* getInode(inode_id_t id) {
    if (id < 0 || id >= curr) {
        printf("Invalid inode ID: %d\n", id);
        return NULL;
    }
    return &inodes[id];
}


void deleteInode(struct Inode* inode) {
    printf("DEL INODE %d\n", inode->id);

    free(inode->name);
    if (inode->data) {
        free(inode->data);
    }
    inode->name = NULL;
    inode->type = 0;
    inode->data = NULL;
    inode->id = -1;
    inode->permissions = 0;
    inode->blocks = 0;
    inode->created = 0; 
    inode->modified = 0; 
    inode->last_access = 0;
    inode->next = free_inode;
    free_inode = inode;
}


static int curr = 1;
struct Inode* createInode(char* name, enum InodeType type) {
    struct Inode* inode;
    if (free_inode) {
        inode = free_inode;
        free_inode = free_inode->next;
    } else {
        inode = &inodes[curr++];
    }
    inode->name = strdup(name);
    if (!inode->name) {
        perror("Failed to allocate memory for inode name");
        return NULL;
    }
    inode->type = type;
    inode->data = NULL;
    inode->permissions = (type == I_DIR) ? 0755 : 0644;
    inode->blocks = 1;
    inode->created = time(NULL); 
    inode->modified = inode->created; 
    inode->last_access = inode->created; 
    inode->next = NULL;    
    return inode;
}