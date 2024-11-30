#include "./inodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cant_inodes = 0;

struct Inode* root_inode= NULL;
static int new_inodo_id = 1;

static struct Inode * setBaseInode(int id){
    struct Inode * inode = &inodes[id];
    
    inode->id = id;    
    return inode;
}


void serializeInodes(struct SerialFD* fd_out){    // Persona 4/1?
    printf("SERIALIZE inodes fd: %d \n",fd_out->fd);

    for (int i = 0; i < cant_inodes; i++) {
        struct Inode * inode = &inodes[i];
        if(inode->id == -1){
            continue;
        }
        
        writeInt(fd_out, cant_inodes);
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

}

void deserializeInodes(struct SerialFD* fd_in){  // Persona 4/1?
    printf("DESERIALIZE inodes fd: %d \n",fd_in->fd);

    new_inodo_id = 0;
    free_inode = setBaseInode(1);
    int res = readInt(fd_in, &cant_inodes);

    for (int i = 0; i < cant_inodes; i++) {
        int *id = 0;
        res = readInt(fd_in, id);
        if (res == -1) {
            return;
        }

        struct Inode * inode = setBaseInode(*id);

        res = readStr(fd_in, &inode->name);
        res = readInt(fd_in, (int*) &inode->type);
        res = readInt(fd_in, &inode->size_bytes);
        res = readInt(fd_in, &inode->blocks);
        res = readInt(fd_in, &inode->first_block);
        res = readInt(fd_in, &inode->permissions);
        res = readInt(fd_in, (int *) &inode->created);
        res = readInt(fd_in, (int *) &inode->modified);
        res = readInt(fd_in, (int*) &inode->last_access);
        
        new_inodo_id++;
    }
}

void initInodes(){
    root_inode = setBaseInode(0);
    free_inode = setBaseInode(1);
    new_inodo_id = 2;
    
    root_inode->name = "/";
    root_inode->type = I_DIR;
}

void statOf(struct Inode* inode, struct stat *st){ // Persona 4
    printf("STAT OF %s!\n",inode->name);
    
    if (inode->type == I_DIR){
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
    
    inode->name = NULL;
    inode->type = 0;
    
    inode->size_bytes = 0; 
    inode->blocks = 0;
    inode->first_block= -1; 
    
    
    inode->id = -1;
    inode->permissions = 0;
    inode->created = 0; 
    inode->modified = 0; 
    inode->last_access = 0;
    inode->next = free_inode;
    free_inode = inode;
}


struct Inode* createInode(const char* name, enum InodeType type) {
    struct Inode* inode;
    if (free_inode->next == NULL) {
      
        free_inode->next= setBaseInode(new_inodo_id++);
    }
    
    inode = free_inode;
    printf("FREE INODE WAS %d\n",inode->id);
    
    if(setNewName(inode, name) != 0){
        return NULL;
    }
    
    // Pop del free inode.
    free_inode= free_inode->next;
    
    inode->type = type;
    inode->permissions = (type == I_DIR) ? 0755 : 0644;
    
    inode->blocks = 0;
    inode->first_block = NOT_DEFINED_BLOCK;
    inode->size_bytes = 0; 

    inode->created = time(NULL); 
    inode->modified = inode->created; 
    inode->last_access = inode->created; 
    inode->next = NULL;    
    return inode;
}
