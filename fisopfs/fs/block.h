#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE 512
struct Block {
	int id;
	int size;
	char data[BLOCK_SIZE];

	struct Block *next_free;
};

#endif
