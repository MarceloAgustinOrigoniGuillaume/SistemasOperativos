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

// Manejo de archivos

void writeData(struct Inode* file, char* buff, int count){ // Persona 3
    *buff = 0;
    printf("WRITE usl: %s count: %d\n",file->name, count);
}
int readData(struct Inode* file, char* buff_out, int data_off, size_t count){ // Persona 3
    *buff_out = 0;
    printf("READ usl: %s off:%d count: %ld\n",file->name, data_off, count);
    return 0;
}
void freeFile(struct FileData* data){ // Persona 3
    freeBlock(data->first_block);
}

