#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "./filesystem.h"
#include "./blocks.h"
#include "./directories.h"
#include "./inodes.h"
#include <string.h>

#include <stdio.h>

#include <errno.h>

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int getattrs(const char *path, struct stat *st){
     /*
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	} else if (strcmp(path, "/fisop") == 0) {
		st->st_uid = 1818;
		st->st_mode = __S_IFREG | 0644;
		st->st_size = 2048;
		st->st_nlink = 1;
	} else {
		return -ENOENT;
	}

	return 0;
     */
     
     struct Inode* res = searchRelative(path);
     
     if(res == NULL){
         return -ENOENT;
     }
     
     statOf(res, st);
     return 0;
}

int readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi){
	
       printf("[debug] fisopfs_readdir - path: %s\n", path);
       
       struct Inode* res = searchRelative(path);
       
       if(res == NULL){
           return -ENOENT;
       }
	
       // Los directorios '.' y '..'
       filler(buffer, ".", NULL, 0);
       filler(buffer, "..", NULL, 0);

       // Si nos preguntan por el directorio raiz, solo tenemos un archivo
       if (strcmp(path, "/") == 0) {
	    filler(buffer, "fisop", NULL, 0);
	    return 0;
       }
       struct DirEntries children;
       
       readChildren(res, &children);
       struct Inode* child;
       for(int i = 0; i< children.count; i++){
           child = children.first+ i;
           filler(buffer, child->name, NULL, 0);
       }
       

       return 0;

}

int readfile(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi){
	
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	
        struct Inode* res = searchRelative(path);
       
        if(res == NULL){
            return -ENOENT;
        }
        
        return readData(res, buffer, offset, size);
	/*
	// Solo tenemos un archivo hardcodeado!
	if (strcmp(path, "/fisop") != 0)
		return -ENOENT;

	if (offset + size > strlen(fisop_file_contenidos))
		size = strlen(fisop_file_contenidos) - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, fisop_file_contenidos + offset, size);

	return size;
        */        
}

#endif
