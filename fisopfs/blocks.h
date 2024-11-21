#ifndef BLOCKS_H
#define BLOCKS_H
#include "./block.h"

#define BLOCK_COUNT 512

// Persona 3
struct Block blocks[BLOCK_COUNT];
Block * first_free;

Block * getAndUseFree();// Persona 3

void freeBlock(Block * block); // Persona 3

void serializar(int fd_out); // Persona 1
void deserializar(int fd_in); // Persona 1

#endif
