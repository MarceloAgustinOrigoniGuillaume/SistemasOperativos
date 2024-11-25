#define FUSE_USE_VERSION 30

#include "fs/serial.h"
#include <stdio.h>
#include <sys/types.h>

#include <string.h>
#include <stdlib.h>
#include "fs/inode.h"

void showBytes(const char* buff, int count){
    printf("count: %d 0x%02X(%u)",count, (unsigned char)*buff,(unsigned char)*buff);
    
    for(int ind = 1; ind< count; ind++){
        char vl = *(buff+ind);
        printf(", 0x%02X(%u)",(unsigned char)vl, vl);
    }
    printf("\n");
}


static void testUno(const char *filename){
     int err = 0;
     struct SerialFD writer = openWriter(filename, &err);
     
     writeInt(&writer, 25);
     writeStr(&writer, "UNO");
     writeInt(&writer, 50);
     
     closeWriter(&writer);
     
     

     struct SerialFD reader = openReader(filename, &err);
     closeWriter(&reader);
     int out;
     char * outStr = NULL;
     
     readInt(&reader, &out);
     if(out != 25){
         printf("---> FIRST NUM NO ERA 25 fue %d!\n", out);
     }
     
     readStr(&reader, &outStr);
     
     if(outStr == NULL){
         printf("-> out str era null\n");
     } else if(strcmp("UNO", outStr) != 0){
         printf("---> FIRST STR NO ERA 'UNO' fue '%s'\n", outStr);
     }
     
     readInt(&writer, &out);
     if(out != 50){
         printf("---> SECOND NUM NO ERA 50! fue %d!\n", out);
     }

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
     int data = 512+ 254;
     char* buff = "UN ELEMENTO!";
     showBytes(buff, strlen(buff));
     printf("----------\n");
     buff = (char*) &data;
     showBytes(buff, sizeof(data));
     
     
     testUno(argv[1]);
     
     return 0;
     //int ret = writeData(argv[1]);
     //if(ret != 0){
     //    return ret;
     //}
     
     //return showData(argv[1]);
}
