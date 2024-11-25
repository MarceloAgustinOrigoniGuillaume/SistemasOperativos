#define FUSE_USE_VERSION 30

#include "fs/serial.h"
#include <stdio.h>
#include <sys/types.h>

#include <string.h>
#include <stdlib.h>
#include "fs/inode.h"

static void testUno(const char *filename){
     int err = 0;
     struct SerialFD writer = openWriter(filename, &err);
     
     writeInt(&writer, 25);
     writeStr(&writer, "UNO");
     writeInt(&writer, 50);
     
     closeWriter(&writer);
     
     

     struct SerialFD reader = openReader(filename, &err);
     int out;
     char * outStr = NULL;
     
     err = readInt(&reader, &out);
     if(out != 25){
         printf("--->ERR %d FIRST NUM NO ERA 25 fue %d!\n",err, out);
     }
     
     err =readStr(&reader, &outStr);
     
     if(outStr == NULL){
         printf("->ERR %d out str era null\n",err);
     } else if(strcmp("UNO", outStr) != 0){
         printf("--->ERR %d FIRST STR NO ERA 'UNO' fue '%s'\n",err, outStr);
     }
     
     err = readInt(&reader, &out);
     if(out != 50){
         printf("--->ERR %d SECOND NUM NO ERA 50! fue %d!\n",err, out);
     }
     closeWriter(&reader);

}


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


static int testInode(const char * filename){
     printf("TEST INODE?!---------- %s\n",filename);
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
     
     printf("WROTE ALL GOOD?!---------- read: %s\n",filename);
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
     
     testUno(argv[1]);
     return testInode(argv[1]);
}
