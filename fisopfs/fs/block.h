#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE 512
struct Block {
    int tempuseless;
    char data[BLOCK_SIZE];
};

#endif
