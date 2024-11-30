#ifndef BLOCKS_H
#define BLOCKS_H
#include "./block.h"
#include "./inode.h"
#include "./serial.h"
#include <stddef.h>

void initBlocks();

// Para tests!
int writeToBlock(struct Block* block, const char* buff, int off , const int count);

void serializeBlockData(struct SerialFD* fd_out, struct Block* block);
void deserializeBlockData(struct SerialFD* fd_in, struct Block* block);


void serializeBlocks(struct SerialFD* fd_out);    // Persona 3/1?
void deserializeBlocks(struct SerialFD* fd_in);   // Persona 3/1?

#define BLOCK_COUNT 512

// Persona 3
struct Block blocks[BLOCK_COUNT];

int allocFile(struct Inode* file); // Persona 3

// Manejo de archivos

int writeData(struct Inode* file, const char* buff,  int data_off, size_t count); // Persona 3
int readData(struct Inode* file, char* buff_out, int data_off, size_t count); // Persona 3
void freeFile(struct Inode* file); // Persona 3

void truncate(struct Inode* file, int size); // Persona 3

#endif
