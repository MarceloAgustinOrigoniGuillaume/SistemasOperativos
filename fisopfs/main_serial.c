#define FUSE_USE_VERSION 30

#include "fs/serial.h"
#include <stdio.h>
#include <stdlib.h>
#include "fs/inode.h"

static void saveInode(struct SerialFD* writer, struct Inode* inode){
    writeInt(writer, inode->id);
    printf("WROTE ID %d\n", inode->id); 
    writeStr(writer, inode->name);
    writeInt(writer, 44); //Perm?
    
}

static void loadInode(struct SerialFD* writer, struct Inode* inode){
    readInt(writer, &inode->id);
    printf("ID IS %d\n", inode->id); 
    readStr(writer, &inode->name);
    printf("NAME IS %s\n", inode->name); 
    int prm;
    readInt(writer, &prm); //Perm?
    printf("PREM %d\n", prm); 
}


static int writeData(const char * filename){
     printf("ALL GOOD?!---------- %s\n",filename);
     int err = 0;
     struct SerialFD writer = openWriter(filename, &err);
     
     struct Inode inodo;
     
     inodo.name = "INODO 2!";
     inodo.id = 252;
     
     saveInode(&writer, &inodo);
     saveInode(&writer, &inodo);
     
     inodo.name = "INODO2321!";
     inodo.id = 321;
     saveInode(&writer, &inodo);
     closeWriter(&writer);
     
     return 0;
}

static int showData(const char * filename){
     printf("ALL GOOD?! %s\n",filename);
     int err = 0;
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
     int ret = writeData(argv[1]);
     if(ret != 0){
         return ret;
     }
     
     return showData(argv[1]);
}
