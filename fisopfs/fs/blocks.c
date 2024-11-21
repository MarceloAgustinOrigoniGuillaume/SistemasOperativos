#include "./blocks.h"
#include <stdio.h>
void serializeFileData(int fd_out, const struct FileData* data_out){
    printf("SERIALIZE fd: %d , usl: %d\n",fd_out,data_out->first_block->tempuseless);
}
void deserializeFileData(int fd_in, struct FileData* data){
    printf("DESERIALIZE fd: %d , usl: %d\n",fd_in,data->first_block->tempuseless);
}

struct Block * first_free = NULL;

struct Block * getAndUseFree(){
      return first_free; 
}

// Persona 3
void freeBlock(struct Block * block){
    printf("FREE usl: %d\n",block->tempuseless);
    
}

static struct FileData def_file_data;

int allocFile(struct Inode* file){ // Persona 3
    printf("Alloc file: %s\n",file->name);
    file->data = (struct Data *)&def_file_data;
    return 0;
}

// Manejo de archivos

int writeData(struct Inode* file, const char* buff,  int data_off, size_t count){ // Persona 3
    printf("WRITE usl: %s off:%d count: %ld\n",file->name, data_off, count);
    
    const char itm = (*buff);
    printf("first: %c\n", itm);
    return 0;
}
int readData(struct Inode* file, char* buff_out, int data_off, size_t count){ // Persona 3
    printf("READ usl: %s off:%d count: %ld\n",file->name, data_off, count);
    
    const char itm = *buff_out;
    printf("first: %c\n", itm);
    return 0;
}
void freeFile(struct Inode* inode){ // Persona 3
    printf("FREE file: %s\n",inode->name);
    
    //freeBlock((inode->data)->first_block);
}

