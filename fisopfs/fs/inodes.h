#ifndef INODES_H
#define INODES_H

#include <sys/stat.h>

#include "./inode.h"
#define INIT_INODES 256

struct Inode inodes[INIT_INODES];
struct Inode* free_inode;

struct Inode* root_inode;

// Manejo de inodos
struct Inode* createInode(char* name, enum InodeType tipo); // Persona 4
void deleteInode(struct Inode* inode); // Persona 4
void statOf(struct Inode* inode, struct stat *st, int uid); // Persona 4

struct Inode* getInode(inode_id_t id); // Persona 4

#endif
