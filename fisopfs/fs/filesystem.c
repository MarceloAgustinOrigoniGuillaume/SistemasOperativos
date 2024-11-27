#include "./filesystem.h"
#include "./blocks.h"
#include "./directories.h"
#include "./inodes.h"
#include "./serial.h"

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
   root_inode = inode;
   
   allocDir(inode);
   
   inode = createInode("somefile", I_FILE);
   allocFile(inode);
}

int fs_getattrs(const char *path, struct stat *st){
     printf("[debug] fs_getattr - path: %s\n", path);
     
     struct Inode* res = searchRelative(path);
     
     if(res == NULL){
         return -ENOENT;
     }
     
     statOf(res, st);
     return 0;
}

int fs_utimens(const char * path, const struct timespec tv[2]){
    long sec = tv->tv_sec;
    long nsec = tv->tv_nsec;
    
    
    printf("[debug] fs_utimens FOR '%s' %ld %ld\n",path, sec,nsec);
    struct Inode* res = searchRelative(path);
   
    if(res == NULL){
        return -ENOENT;
    }
    printf("[debug] fs_utimens %s EXISTED UPDATE?!\n",path);
    res->modified = nsec;
    res->last_access = nsec;
    
    return 0;    
}





int fs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi){

    printf("[debug] fs_readdir - path: %s\n", path);
    
    struct Inode* res = searchRelative(path);
    
    if (res == NULL){
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

    struct DirEntries *children = malloc(sizeof(struct DirEntries*) * res->data->size);
    
    readChildren(res, children);
    struct Inode* child;
    for(int i = 0; i < res->data->size; i++){
        child = children[i].inode;
        filler(buffer, child->name, NULL, 0);
    }
    
    free(children);

    return 0;

}

static const char * no_data = "No DATA\n";

int fs_readfile(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi){
	printf("Is this fucking function ever called?\n");
	printf("[debug] fs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	
        struct Inode* res = searchRelative(path);
       
        if(res == NULL){
            return -ENOENT;
        }
        printf("Calling readData\n");
        int dt= readData(res, buffer, offset, size);
        printf("Read %d bytes\n",dt);
        if(dt == 0){
           int count = strlen(no_data)- offset;
           if(count <= 0){
               return 0;
           }	   
	   count = size > count  ? count : size;

	   memcpy(buffer, no_data + offset, count);
           return count;        
        }
        
        return dt;
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


int fs_truncate(const char * path, off_t new_size){
    printf("[debug] fisopfs TRUNCATE/RESIZE '%s' to %ld\n",path, new_size);
    struct Inode* res = searchRelative(path);
    if(res == NULL){
        return -ENOENT;
    }
    printf("[debug] RESET DATA TO 0 on '%s'\n",res->name);
        
    return 0;
}


int fs_write(const char *path, const char *buf,
		  size_t size, off_t off, struct fuse_file_info *fi){
	printf("[debug] fisopfs write file %s s: %ld off: %li\n",
	       path, size, off);
        struct Inode* res = searchRelative(path);
        if(res == NULL){
            return -ENOENT;
        }
        
	printf("[debug] found for write!\n");
        int wrote = writeData(res, buf, off, size);
        if(wrote  <0){
            return 0; // Deberia retornar el error especifico.
        }
        
        return wrote;
}


int fs_touch(const char *path, mode_t mode,
		   struct fuse_file_info *fi){
	printf("[debug] fisopfs touch %s %d\n",
	       path, mode);
	
	char * name_child = NULL;
	struct Inode* parent = searchNew(path, &name_child);
	if(parent == NULL || name_child == NULL){
	     return -ENOENT;
	}
	
	struct Inode* child = createInode(name_child, I_FILE);
	if(child == NULL){
	     return -ENOENT;	     
	}
	
	allocFile(child); // Crea datos
	
	
	addChild(parent, child);
    return 0;	     
}

int fs_mkdir(const char *path, mode_t mode//,struct fuse_file_info *fi
                 ){
                 
	printf("[debug] fisopfs mkdir %s %d\n",
	       path, mode);
	       
	char *name_child = NULL;
	struct Inode* parent= searchNew(path, &name_child);
	
	if(parent == NULL || name_child == NULL){
	     return -ENOENT;
	}
	
	struct Inode* child = createInode(name_child, I_DIR);

	free(name_child);

	if(child == NULL){
	     return -ENOENT;	     
	}
	
	allocDir(child); // Crea directorio
	addChild(parent, child);

    printf("Created directory %s !! \n", child->name);
    return 0;	     
}

int fs_rmdir(const char *path){
	printf("[debug] fisopfs rmdir %s\n",
	       path);
	       
	struct Inode* child = rmChild(path);
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
	       
	struct Inode* child = rmChild(path);
	if(child == NULL){
            return -ENOENT;
        }
        
        freeFile(child); // Libera la data del inodo.
        deleteInode(child); // Libera el inodo en si.
        
        return 0;
}


void* fs_init(struct fuse_conn_info *conn){
	printf("[debug] fisopfs init from %s\n",filedisk);
	int err = 0;
	struct SerialFD fd = openReader(filedisk, &err);
	
	if(err != 0){
	    printf("Serialize open file for deserializing failed! %d '%s'\n", err,filedisk);
	    hardcodefs();
	    return NULL;
	}
	
	deserializeBlocks(&fd);
	deserializeInodes(&fd);

	closeWriter(&fd);
	hardcodefs();
	return NULL;
}

void fs_destroy(){
	printf("[debug] fisopfs serialize to %s!\n",filedisk);
	int err= 0;
	struct SerialFD fd = openWriter(filedisk, &err);
	
	if(err != 0){
	    printf("Serialize open file for serializing failed! %d '%s'\n",err, filedisk);
	    return;
	}
	
	serializeBlocks(&fd);
	serializeInodes(&fd);
	
	closeWriter(&fd);
}
