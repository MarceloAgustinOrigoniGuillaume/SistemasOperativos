#include "./directories.h"
#include "./inodes.h"
#include <stdio.h>
#include <string.h>

struct Inode* searchRelative(const char* path){ // Persona 2
    printf("LOOK FOR %s\n", path);
    if(strcmp(path, root_inode.name) == 0){
         printf("WAS ROOT?%s vs %s\n",path, root_inode.name);

         return &root_inode;
    }
    printf("WAS NOT ROOT? '%s' vs '%s' \n",path, root_inode.name);
    
    return NULL;
}

void readChildren(struct Inode* dir, struct DirEntries* out){ // Persona 2
    printf("Get Children of %s\n", dir->name);
    if(strcmp(dir->name, root_inode.name) == 0){
        out->count = 1;
        out->first = getinode(0);
        
        return;
    }
    out->count = 0;
}
