#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <fuse.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

void hardcodefs();

void serialize(int fd_out);    // Persona 1
void deserialize(int fd_in);   // Persona 1

int getattrs(const char *path, struct stat *st); // Persona 1

int readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi);

int readfile(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi);
#endif
