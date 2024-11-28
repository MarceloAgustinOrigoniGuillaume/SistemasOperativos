#include "./inodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void serializeInodes(struct SerialFD* fd_out){    // Persona 4/1?
    printf("SERIALIZE inodes fd: %d \n",fd_out->fd);

}
void deserializeInodes(struct SerialFD* fd_in){  // Persona 4/1?
    printf("DESERIALIZE inodes fd: %d \n",fd_in->fd);
}

struct Inode* root_inode= NULL;
static int new_inodo_id = 1;

void statOf(struct Inode* inode, struct stat *st){ // Persona 4
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
        st->st_size = inode->data ? inode->data->size+10 : 10;
    }

    st->st_uid = inode->id;
    st->st_blocks = inode->blocks;
    st->st_atime = inode->last_access; 
    st->st_mtime = inode->modified; 
    st->st_ctime = inode->created;
}

struct Inode* getinode(inode_id_t id) {
    if (id < 0 || id >= new_inodo_id) {
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


struct Inode* createInode(const char* name, enum InodeType type) {
    struct Inode* inode;
    if (free_inode) {
        inode = free_inode;
        free_inode = free_inode->next;
    } else {
        inode = &inodes[new_inodo_id++];
    }
    
    int count = name? strlen(name) : 0;
    if(count <= 0){
       perror("Invalid name for new inode\n");
       return NULL;
    }
    
    inode->name = (char *) malloc(sizeof(char) * count+ 1);
    
    if (!inode->name) {
        perror("Failed to allocate memory for inode name\n");
        return NULL;
    }
    
    memcpy(inode->name, name, count);    
    *(inode->name+count) = 0;
    
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
