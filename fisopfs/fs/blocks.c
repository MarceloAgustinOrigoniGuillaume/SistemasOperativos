#include "./blocks.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cant_blocks = 0;

static int writeToBlock(struct Block* block, const char* buff, int off , const int count){
    if(off > block->size){
        return -1; // Se fallo escritura.
    }
    
    int max = BLOCK_SIZE- off;
    if(count < max){
        max = count;
    }
    if(max == 0){
        return 0;
    }
    
    memcpy(&block->data[off], buff, max);
    
    int nw_size = off+max;
    if(nw_size > block->size){
        block->size = nw_size;
    }
    
    return max;
}

static int readFromBlock(struct Block* block, char* buff, int off , const int count){
    if(off > block->size){
        return -1; // Se fallo escritura.
    }
    
    int max = block->size - off;
    if(count < max){
        max = count;
    }
    if(max == 0){
        return 0;
    }
    
    memcpy(buff, &block->data[off], max);
    
    return max;
}

static struct Block * first_free;
static int new_block; 


static struct Block * resetBlock(int id){
    struct Block * block = &blocks[id];
    
    block->id = id;
    block->size = 0;
    block->next_free = NULL;
    
    return block;
}

static struct Block * getBlock(int id){
    if(id < 0 || id >= new_block){
         return NULL;
    }
    
    return &blocks[id];
}

void freeBlock(int id){
    struct Block * prev_free = first_free;
    struct Block * next_free = first_free->next_free;
        
    while(next_free){
        
        if(prev_free->id < id && next_free->id > id){
              //Add in between
              struct Block * curr = resetBlock(id);
              prev_free->next_free = curr;
              curr->next_free = next_free;      
              cant_blocks--; // Reduzco la cantidad de bloques        
              return;
        }
        
        next_free = next_free->next_free;
    }
    
    // Se llego al final? por lo que se debe agregar al final como nuevo free
    prev_free->next_free = resetBlock(id); // Agrega a la lista de frees el bloque curr
    
}

static int getFreeBlock(){
    int id = first_free->id;
    
    first_free->size = 0;
    
    if(first_free->next_free == NULL){
         // Check de capacidad?
         first_free->next_free = resetBlock(new_block++);
    }
    
    first_free = first_free->next_free; // Pop
    
    return id;
}



void initBlocks(){
    first_free = resetBlock(0);
    
    new_block = 1;
}
void serializeBlocks(struct SerialFD* fd_out){
    printf("SERIALIZE blocks fd: %d \n",fd_out->fd);

    for (int i = 0; i < cant_blocks; i++) {
        struct Block * block = getBlock(i);
        if(block == NULL){
            continue;
        }
        
        writeInt(fd_out, block->id);
        writeInt(fd_out, block->size);
        writeMsg(fd_out, block->data, BLOCK_SIZE);
    }
}


void deserializeBlocks(struct SerialFD* fd_in){
    printf("DESERIALIZE blocks fd: %d\n",fd_in->fd);

    new_block = 0;
    first_free = resetBlock(0);
    int res = readInt(fd_in, &cant_blocks);

    for (int i = 0; i < cant_blocks; i++) {
        int *id = 0;        
        res = readInt(fd_in, id);
        if (res == -1) {
            return;
        }

        struct Block * block = resetBlock(*id);
        res = readInt(fd_in, &block->size);
        res = readAll(fd_in->fd, (uint8_t*)block->data, BLOCK_SIZE);
        
        new_block++;
    }
}


// Persona 3
// static struct FileData def_file_data;

int allocFile(struct Inode* file){ // Persona 3
    printf("Alloc file: %s to id %d\n",file->name,first_free->id);
    
    file->blocks = 1;
    file->size_bytes = 0;
    file->first_block = getFreeBlock();
    cant_blocks++; // Aumento la cantidad de bloques
    return 0;
}

// Manejo de archivos

int writeData(struct Inode* file, const char* buff,  int data_off, size_t count){ // Persona 3
    
    printf("WRITE usl: %s off:%d count: %ld\n",file->name, data_off, count);
    
    struct Block* block = getBlock(file->first_block);
    
    if(block == NULL){
        printf("Unalloced block for file write!\n");
        return -1; // Not all
    }    
    
    printf("At write size is: %d\n",block->size);

    int written = writeToBlock(block, buff,data_off, count);
    
    // Agrega al inodo size
    if(written > 0){
        file->size_bytes+= written;
    }
    
    return written;
}
int readData(struct Inode* file, char* buff_out, int data_off, size_t count){ // Persona 3
    printf("READ usl: %s off:%d count: %ld\n",file->name, data_off, count);
    printf("The inode size is: %d\n",file->size_bytes);
    
    struct Block* block = getBlock(file->first_block);    
    if(block == NULL){
        printf("Unalloced block for file read!\n");
        return -1; // Not all
    }
    
    printf("The first block is not null, its correctly allocated\n");
    return readFromBlock(block, buff_out, data_off, count);
}


void freeFile(struct Inode* inode){ // Persona 3
    printf("FREE file: %s block_first: %d\n",inode->name,inode->first_block);
    
    if(inode->first_block>=0){
        freeBlock(inode->first_block);
    }
}

void truncate(struct Inode* file, int size){ // Persona 3
    struct Block* block = getBlock(file->first_block);    
    if(block == NULL){
        printf("Unalloced block for file truncate!\n");
        return; // Not all
    }

    block->size = size > BLOCK_SIZE? BLOCK_SIZE: size;
    
}
