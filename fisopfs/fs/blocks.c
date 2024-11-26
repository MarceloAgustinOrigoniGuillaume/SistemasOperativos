#include "./blocks.h"
#include <stdio.h>
#include <stdlib.h>

void serializeBlocks(struct SerialFD* fd_out){
    printf("SERIALIZE blocks fd: %d \n",fd_out->fd);
}
void deserializeBlocks(struct SerialFD* fd_in){
    printf("DESERIALIZE blocks fd: %d\n",fd_in->fd);
}


void serializeFileData(struct SerialFD* fd_out, const struct FileData* data_out){
    printf("SERIALIZE fd: %d , usl: %d\n",fd_out->fd,data_out->first_block->tempuseless);
}
void deserializeFileData(struct SerialFD* fd_in, struct FileData* data){
    printf("DESERIALIZE fd: %d , usl: %d\n",fd_in->fd,data->first_block->tempuseless);
}

struct Block * first_free = NULL;

struct Block * getAndUseFree(){
      return first_free; 
}

// Persona 3
void freeBlock(struct Block * block){
    printf("FREE usl: %d\n",block->tempuseless);
    
}

// static struct FileData def_file_data;

int allocFile(struct Inode* file){ // Persona 3
    printf("Alloc file: %s\n",file->name);
    struct FileData* data = malloc(sizeof(struct FileData));
    data->first_block = malloc(sizeof(struct Block));
    data->size = 0;
    file->data = (struct Data *)data;
    printf("Alloced file data\n");
    return 0;
}

// Manejo de archivos

int writeData(struct Inode* file, const char* buff,  int data_off, size_t count){ // Persona 3
    printf("WRITE usl: %s off:%d count: %ld\n",file->name, data_off, count);
    printf("The file size is: %d\n",file->data->size);

    struct FileData* data = (struct FileData*)file->data;

    const char itm = (*buff);
    printf("first: %c\n", itm);

    if(data->first_block == NULL){
        // Unalloced file data, it shouldn't happen i think
        printf("Unalloced file data\n");
        return -1;
    }

    printf("The first block is not null, its correctly allocated\n");

    int written_bytes = 0;

    for (int i = 0; i < count; i++){
        printf("Writing byte %d\n", i);
        if(data_off + i >= BLOCK_SIZE) {
            // Block full
            printf("Block full\n");
            file->data->size += written_bytes;
            return written_bytes;
        }
        written_bytes++;
        data->first_block->data[data_off + i] = buff[i];
    }

    file->data->size += written_bytes;
    printf("The file size is now: %d\n",file->data->size);
    printf("Wrote %d bytes\n", written_bytes);
    return written_bytes;
}
int readData(struct Inode* file, char* buff_out, int data_off, size_t count){ // Persona 3
    printf("READ usl: %s off:%d count: %ld\n",file->name, data_off, count);
    printf("The file size is: %d\n",file->data->size);
    printf("The offset received is: %d\n",data_off);
    struct FileData* data = (struct FileData*)file->data;

    if(data->first_block == NULL){
        // Unalloced file data, it shouldn't happen i think
        printf("Unalloced file data\n");
        return -1;
    }

    printf("The first block is not null, its correctly allocated\n");

    int read_bytes = 0;

    for (int i = 0; i < count; i++){
        printf("Reading byte %d\n", i);
        if(data_off + i >= data->size) {
            // Block full
            printf("Trying to read out of bounds\n");
            break;
        }
        read_bytes++;
        buff_out[i] = data->first_block->data[data_off + i];
    }

    printf("Read %d bytes\n", read_bytes);
    printf("The data read is: %s\n", buff_out);
    return read_bytes;
}
void freeFile(struct Inode* inode){ // Persona 3
    printf("FREE file: %s\n",inode->name);

    struct FileData* data = (struct FileData*)inode->data;
    free(data->first_block);
    free(data);
    
    //freeBlock((inode->data)->first_block);
}

