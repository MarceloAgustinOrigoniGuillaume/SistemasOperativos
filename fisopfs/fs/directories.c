#include "./directories.h"
#include "./inodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EXIT_FAILURE 1
#define ROOT_DIR "/"
static int exists_def = 0; 

char *strdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = malloc(len);
    if (dst) {
        memcpy(dst, src, len);
    }
    return dst;
}

static struct DirData def_data;

int allocDir(struct Inode* dir){ // Persona 2
    printf("ALLOC DIR %s\n", dir->name);
    dir->data = (struct Data *)&def_data;
    return 0;
}

// Libera al directorio, recursivamente
void freeDir(struct Inode* dir){ // Persona 2
    printf("FREE DIR %s\n", dir->name);

    if (dir->type != I_DIR) {
        deleteInode(dir);
        return;
    }

    for (int i = 0; i < dir->data->size; i++) {
        struct DirData *data = dir->data;
        struct Inode* child = data->entries[i].inode;
        if (child) {
            freeDir(child);
        }
    }
}

// Busca al inodo, y lo remueve del padre. Retorna el inodo hijo. Para su posterior liberacion de hacer falta.
struct Inode* rmChild(const char* path){ // Persona 2
    printf("RM CHILD root %s \n", path);

    int count;
    char **directories = split(path, "/", &count);
    char *parent_path = malloc(strlen(path) + 1);
    for (int i = 0; i < count - 1; i++) {
        strcat(parent_path, "/");
        strcat(parent_path, directories[i]);
    }

    struct Inode* parent = searchRelative(parent_path);
    struct Inode* child = searchRelative(path);

    if (!parent) {
        printf("NO PARENT FOR %s\n", path);
        return NULL;
    }

    if (!child) {
        printf("NO CHILD FOR %s\n", path);
        return NULL;
    }

    for (int i = 0; i < parent->data->size; i++) {
        struct DirData *data = parent->data;
        if (data->entries[i].inode == child) { 
            data->entries[i].inode = NULL; 
            data->size--;
            return child;
        }
    }

    free(directories);
    free(parent_path);

    return NULL;
}

// Agrega el inodo hijo como hijo al padre.
int addChild(struct Inode* parent, struct Inode* child){ // Persona 2
    printf("ADD CHILD parent %s child: %s\n",parent->name, child->name);

    struct DirData *data = parent->data;
    if (data->size == data->capacity) {
        perror("No hay espacio para agregar un nuevo hijo");
        return -1;
    }

    struct DirEntries* entries = data->entries; 
    for (int i = 0; i < parent->data->size; i++) {
        if (entries[i].inode == NULL) { 
            entries[i].inode = child;
            parent->data->size++;
            return 0;
        }
    }

    return 0;
}

struct Inode* searchRelative(const char* path){ // Persona 2
    printf("LOOK FOR %s\n", path);

    if(strcmp(path, ROOT_DIR) == 0){
        return getinode(0);
    }

    int count;
    char **directories = split(path, "/", &count);
    char *target = directories[count - 1];
    
    struct Inode* current = getinode(0);
    for (int i = 1; i < count; i++) {
        if (current->type != I_DIR) {
            return current;
        }

        struct DirEntries children[INIT_DIR_ENTRIES];
        readChildren(current, &children);

        for(int j = 0; j < current->data->size; j++){
            char *name = children[j].inode->name;
            if(strcmp(name, directories[i]) == 0){
                current = children[j].inode;
                if (name == target) {
                    free(directories); 
                    return current;
                }
                break;
            }
        }
    }

    printf("WAS NOT EXISTENT? '%s' \n",path);

    free(directories);
    return NULL;
}

char * NAME_DEF = "newfile";

// Retorna el padre! Null si no existe un padre. name_child = Null en caso de que ya exista.
struct Inode* searchNew(const char* path, char ** name_child){ // Persona 2
    int count;
    char **directories = split(path, "/", &count);
    char *target = strdup(directories[count - 1]);
    char *parent_path = malloc(strlen(path) + 1);
    parent_path[0] = '\0';

    for (int i = 0; i < count - 1; i++) {
        strcat(parent_path, "/");
        strcat(parent_path, directories[i]);
    }

    free(directories);

    struct Inode* parent = searchRelative(parent_path);
    struct Inode* child = searchRelative(path);

    free(parent_path);

    if (!parent) {
        printf("NO PARENT FOR %s\n", path);
        free(target);
        return NULL;
    }

    if (child) {
        printf("GOT NAME FOR %s %s\n", target, child->name);
        free(target);
        return parent;
    }

    *name_child = target;
    return parent;
}

void readChildren(struct Inode* dir, struct DirEntries* out){ // Persona 2
    struct DirData *data = dir->data;
    for (int i = 0; i < data->size; i++) {
        if (data->entries[i].inode) {
            out[i] = data->entries[i];
        }
    }
}


char** split(const char* str, const char* delimiter, int* count) {
    char* temp = strdup(str); 

    int parts = 0;
    char* token = strtok(temp, delimiter);
    while (token != NULL) {
        parts++;
        token = strtok(NULL, delimiter);
    }
    free(temp); 

    char** result = malloc(parts * sizeof(char*));
    if (!result) {
        perror("Error reservando memoria");
        exit(EXIT_FAILURE);
    }

    temp = strdup(str); 

    token = strtok(temp, delimiter);
    int i = 0;
    while (token != NULL) {
        result[i] = strdup(token);

        if (!result[i]) {
            perror("Error reservando memoria para token");
            exit(EXIT_FAILURE);
        }

        i++;
        token = strtok(NULL, delimiter);
    }

    free(temp); 
    *count = parts;
    return result;
}

