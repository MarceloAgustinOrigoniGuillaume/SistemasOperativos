#include "./directories.h"
#include "./inodes.h"
#include <stdio.h>
#include <string.h>



static int exists_def = 0; 

static struct DirData def_data;

int allocDir(struct Inode* dir){ // Persona 2
    printf("ALLOC DIR %s\n", dir->name);
    dir->data = (struct Data *)&def_data;
    return 0;
}
// Libera al directorio, recursivamente
void freeDir(struct Inode* dir){ // Persona 2
    printf("FREE DIR %s\n", dir->name);
}

// Busca al inodo, y lo remueve del padre. Retorna el inodo hijo. Para su posterior liberacion de hacer falta.
struct Inode* rmChild(struct Inode* root, const char* path){ // Persona 2
    printf("RM CHILD root %s rel: %s\n",root->name, path);
    return NULL;
}
// Agrega el inodo hijo como hijo al padre.
int addChild(struct Inode* parent, struct Inode* child){ // Persona 2
    printf("ADD CHILD parent %s child: %s\n",parent->name, child->name);
    //-ENOENT
    return 0;
}


struct Inode* searchRelative(const struct Inode* root,const char* path){ // Persona 2
    printf("LOOK FOR %s\n ", path);
    if(strcmp(path, root->name) == 0){
         return getinode(root->id);
    } else if(strcmp(path, "/somefile") == 0){
         printf("WAS SOME FILE!\n");
         return getinode(1);
    }
    
    printf("WAS NOT EXISTENT? '%s' vs '%s' \n",path, root->name);
    
    return NULL;
}


char * NAME_DEF = "newfile";
// Retorna el padre! Null si no existe un padre. name_child = Null en caso de que ya exista.
struct Inode* searchNew(const struct Inode* root, const char* path, char ** name_child){ // Persona 2
    if(exists_def == 0 && strcmp(path, "/newfile") == 0){
         printf("GOT NAME FOR %s %d\n", NAME_DEF, root->id);
         *name_child = NAME_DEF;
         exists_def = 1;
         printf("AFT NAME FOR %s %d\n", NAME_DEF, root->id);
         return getinode(0);
    }
    
    return NULL;
}



void readChildren(struct Inode* dir, struct DirEntries* out){ // Persona 2
    printf("Get Children of %s\n", dir->name);
    if(strcmp(dir->name, "/") == 0){
        if(exists_def == 1){
            out->count = 1;
            printf("Get ind 2... newfile\n");
            out->first = getinode(2);
            return;
        }
        printf("Get ind 1... somefile\n");
        out->count = 1;
        out->first = getinode(1);
        
        return;
    }
    out->count = 0;
}
