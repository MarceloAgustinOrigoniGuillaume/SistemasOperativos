#ifndef INODES_H
#define INODES_H

#include <sys/stat.h>

#include "./inode.h"
#define INIT_INODES 256

struct Inode inodes[INIT_INODES];
struct Inode* free_inode;

// Manejo de inodos
struct Inode* createInode(struct Inode* parent, const char* path, enum InodeType tipo); // Persona 4
void deleteInode(struct Inode* inode); // Persona 4
void statOf(struct Inode* inode, struct stat *st); // Persona 4

struct Inode* getinode(inode_id_t id); // Persona 4

#endif
