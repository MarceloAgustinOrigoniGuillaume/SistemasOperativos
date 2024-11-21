#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "./filesystem.h"
#include "./blocks.h"
#include "./directories.h"
#include "./inodes.h"

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void hardcodefs(){
   struct Inode* inode = &inodes[0];
   inode->name = "somefile";
   inode->type = I_FILE;
   inode->data = NULL;
}

int getattrs(const char *path, struct stat *st){
     printf("[debug] fisopfs_getattr - path: %s\n", path);
     
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
           // Si nos preguntan por el directorio raiz, solo tenemos un archivo
           if (strcmp(path, "/") == 0) {
                 // Los directorios '.' y '..'
                 filler(buffer, ".", NULL, 0);
                 filler(buffer, "..", NULL, 0);
	        filler(buffer, "fisop", NULL, 0);
	        return 0;
           }       
           return -ENOENT;
       }
	
       // Los directorios '.' y '..'
       filler(buffer, ".", NULL, 0);
       filler(buffer, "..", NULL, 0);

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
