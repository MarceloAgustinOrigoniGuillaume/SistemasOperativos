#ifndef INODES_H
#define INODES_H

#include "./inodes.h"

int getattrs(const char *path, struct stat *st); // Persona 1
int readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi);

void deleteInode(Inode* inode); // Persona 4


// Manejo de directorios
Inode* searchRelative(char* path); // Persona 2
void addChild(Inode* parent, Inode* child); // Persona 2
struct DirEntry* readChildren(Inode* dir); // Persona 2
void freeDir(Inode* dir); // Persona 2


// Manejo de archivos
void writeData(Inode* file, char* buff, int count); // Persona 3
void readData(Inode* file, char* buff_out, int count); // Persona 3
void freeFile(Inode* dir); // Persona 2






#endif
