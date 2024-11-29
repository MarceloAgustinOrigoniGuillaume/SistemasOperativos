#ifndef INODES_H
#define INODES_H

#include <sys/stat.h>
#include "./serial.h"

#include "./inode.h"
#define INIT_INODES 256

struct Inode inodes[INIT_INODES];
struct Inode* free_inode;

struct Inode* root_inode;


void initInodes();
void serializeInodes(struct SerialFD* fd_out);    // Persona 4/1?
void deserializeInodes(struct SerialFD* fd_in);   // Persona 4/1?


// Manejo de inodos
struct Inode* createInode(const char* name, enum InodeType tipo); // Persona 4
void deleteInode(struct Inode* inode); // Persona 4
void statOf(struct Inode* inode, struct stat *st); // Persona 4

struct Inode* getinode(inode_id_t id); // Persona 4

#endif
