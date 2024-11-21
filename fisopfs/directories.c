#include "./directories.h"
#include <stdio.h>
struct Inode* searchRelative(const char* path){ // Persona 2
    printf("LOOK FOR %s\n", path);
    return NULL;
}

void readChildren(struct Inode* dir, struct DirEntries* out){ // Persona 2
    out->count = 0;
    printf("Get Children of %s\n", dir->name);
}
