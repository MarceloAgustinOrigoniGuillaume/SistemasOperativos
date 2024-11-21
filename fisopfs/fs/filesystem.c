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

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"
char *filedisk = DEFAULT_FILE_DISK;

//#define MAX_CONTENIDO 100
//static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";


void hardcodefs(){
   struct Inode* inode = &inodes[0];
   
   inode->id = 0;
   inode->name = "/";
   inode->type = I_DIR;
   inode->data = NULL;
   
   root_inode = inode;
   
   
   inode = &inodes[1];
   inode->id = 1;
   inode->name = "somefile";
   inode->type = I_FILE;
   inode->data = NULL;
}

int fs_getattrs(const char *path, struct stat *st){
     printf("[debug] fs_getattr - path: %s\n", path);
     
     struct Inode* res = searchRelative(root_inode, path);
     
     if(res == NULL){
         return -ENOENT;
     }
     
     statOf(res, st);
     return 0;
}

int fs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi){
	
       printf("[debug] fs_readdir - path: %s\n", path);
       
       struct Inode* res = searchRelative(root_inode, path);
       
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

int fs_readfile(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi){
	
	printf("[debug] fs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	
        struct Inode* res = searchRelative(root_inode, path);
       
        if(res == NULL){
            return -ENOENT;
        }
        
        readData(res, buffer, offset, size);
        
        return -ENOENT; 
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




int fs_write(const char *path, const char *buf,
		  size_t size, off_t off, struct fuse_file_info *fi){
	printf("[debug] fisopfs write file %s s: %ld off: %li\n",
	       path, size, off);
        struct Inode* res = searchRelative(root_inode, path);
        if(res == NULL){
            return -ENOENT;
        }
        
        writeData(res, buf, off, size);
        
        return -ENOENT;
}


int fs_touch(const char *path, mode_t mode,
		   struct fuse_file_info *fi){
	printf("[debug] fisopfs touch %s %d\n",
	       path, mode);
	
	char * name_child = NULL;
	struct Inode* parent= searchNew(root_inode, path, &name_child);
	if(parent == NULL || name_child == NULL){
	     return -ENOENT;
	}
	
	struct Inode* child = createInode(name_child, I_FILE);
	if(child == NULL){
	     return -ENOENT;	     
	}
	
	allocFile(child); // Crea datos
	
	
	addChild(parent, child);
        return -ENOENT;	     
}

int fs_mkdir(const char *path, mode_t mode//,struct fuse_file_info *fi
                 ){
                 
	printf("[debug] fisopfs mkdir %s %d\n",
	       path, mode);
	       
	char * name_child = NULL;
	struct Inode* parent= searchNew(root_inode, path, &name_child);
	
	if(parent == NULL || name_child == NULL){
	     return -ENOENT;
	}
	
	struct Inode* child = createInode(name_child, I_DIR);
	
	if(child == NULL){
	     return -ENOENT;	     
	}
	
	allocDir(child); // Crea directorio
	addChild(parent, child);
        return -ENOENT;	     
}

int fs_rmdir(const char *path){
	printf("[debug] fisopfs rmdir %s\n",
	       path);
	       
	struct Inode* child = rmChild(root_inode, path);
	if(child == NULL){
            return -ENOENT;
        }
        
        freeDir(child); // Libera la data del inodo.
        deleteInode(child); // Libera el inodo en si.
        return 0;
}


int fs_unlink(const char* path){
	printf("[debug] fisopfs unlink %s\n",
	       path);
	       
	struct Inode* child = rmChild(root_inode, path);
	if(child == NULL){
            return -ENOENT;
        }
        
        freeFile(child); // Libera la data del inodo.
        deleteInode(child); // Libera el inodo en si.
        
        return 0;
}


void* fs_init(struct fuse_conn_info *conn){
	printf("[debug] fisopfs init from %s\n",filedisk);
	hardcodefs();
	return NULL;
}

void fs_destroy(){
	printf("[debug] fisopfs serialize to %s!\n",filedisk);
}
