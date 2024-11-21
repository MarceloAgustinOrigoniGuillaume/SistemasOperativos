#ifndef BLOCKS_H
#define BLOCKS_H
#include "./block.h"
#include "./inode.h"
#include <stddef.h>


struct FileData{ // Persona 3
   int size; // Campo del struct Data. Tiene que estar arriba de todo!
   struct Block* first_block;
};

void serializeFileData(int fd_out, const struct FileData* data_out);    // Persona 3/1?
void deserializeFileData(int fd_in, struct FileData* data);   // Persona 3/1?


#define BLOCK_COUNT 512

// Persona 3
struct Block blocks[BLOCK_COUNT];
struct Block * first_free;

struct Block * getAndUseFree();// Persona 3

void freeBlock(struct Block * block); // Persona 3

// Manejo de archivos

int writeData(struct Inode* file, const char* buff,  int data_off, size_t count); // Persona 3
int readData(struct Inode* file, char* buff_out, int data_off, size_t count); // Persona 3
void freeFile(struct Inode* file); // Persona 3

#endif
