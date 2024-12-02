#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <fuse.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

char *filedisk;

void hardcodefs();

void serialize(int fd_out);   // Persona 1
void deserialize(int fd_in);  // Persona 1

int fs_getattrs(const char *path, struct stat *st);  // Persona 1
int fs_utimens(const char *path, const struct timespec tv[2]);

int fs_readdir(const char *path,
               void *buffer,
               fuse_fill_dir_t filler,
               off_t offset,
               struct fuse_file_info *fi);

int fs_readfile(const char *path,
                char *buffer,
                size_t size,
                off_t offset,
                struct fuse_file_info *fi);


int fs_touch(const char *path, mode_t mode, struct fuse_file_info *fi);


int fs_truncate(const char *path, off_t new_size);

int fs_write(const char *path,
             const char *buf,
             size_t size,
             off_t off,
             struct fuse_file_info *fi);

int fs_mkdir(const char *path, mode_t mode);

int fs_rmdir(const char *path);


int fs_unlink(const char *path);


void *fs_init(struct fuse_conn_info *conn);
void fs_destroy();

#endif
