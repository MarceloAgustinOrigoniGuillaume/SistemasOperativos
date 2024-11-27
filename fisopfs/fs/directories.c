#include "./directories.h"
#include "./inodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EXIT_FAILURE 1
#define ROOT_DIR "/"
#define NULL_CHAR 1

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
    struct DirData *dir_data = malloc(sizeof(struct DirData));

    dir_data->size = 0;
    dir_data->capacity = INIT_DIR_ENTRIES;
    for (int i = 0; i < INIT_DIR_ENTRIES; i++) {
        dir_data->entries[i].inode = NULL;
    }

    dir->data = (struct Data *) dir_data;
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
int addChild(struct Inode* parent, struct Inode* child) { // Persona 2
    printf("ADD CHILD parent %s child: %s\n", parent->name, child->name);

    struct DirData *data = parent->data; // Asegúrate de trabajar con un puntero correcto
    if (data->size == data->capacity) {
        perror("No hay espacio para agregar un nuevo hijo");
        return -1;
    }

    struct DirEntries* entries = data->entries;
    for (int i = 0; i < data->capacity; i++) { // Iterar sobre toda la capacidad
        if (entries[i].inode == NULL) {
            entries[i].inode = child; // Agregar el hijo
            data->size++;            // Incrementar el tamaño
            return 0;
        }
    }

    return -1;
}



struct Inode* searchChild(struct Inode* dir, const char* name){
    printf("SEARCH CHILD: LOOK FOR %s IN %s\n", name, dir->name);
    struct DirEntries children[dir->data->size];
    readChildren(dir, &children);


    for(int i = 0; i < dir->data->size; i++){
        if(strcmp(children[i].inode->name, name) == 0){
            printf("FOUND %s\n", name);
            return children[i].inode;
        }
    }

    return NULL;
}

struct Inode* searchRelative(const char* path){ // Persona 2
    printf("SEARCH RELATIVE: LOOK FOR %s\n", path);

    if(strcmp(path, ROOT_DIR) == 0){
        return getinode(0);
    }

    int count;
    char **directories = split(path, "/", &count);
    char *target = directories[count - 1];

    printf("COUNT %d\n", count);
    printf("TARGET %s\n", target);

    struct Inode* current = getinode(0);

    if (count == 1) {
        return searchChild(current, target);
    } else {
        for (int i = 0; i < count; i++) {
            printf("LOOK FOR %s\n", directories[i]);
            if (current->type != I_DIR) {
                return current;
            }

            struct Inode* child = searchChild(current, directories[i]);
            if (!child) {
                printf("NOT FOUND %s\n", directories[i]);
                free(directories);
                return NULL;
            }

            printf("FOUND %s, TARGET %s\n", child->name, target);
            if (strcmp(child->name, target) == 0) {
                free(directories);
                printf("FOUND %s -> RETURN SEARCH RELATIVE\n", path);
                return child;
            }

            current = child;
        }
    }

    printf("WAS NOT EXISTENT? '%s' \n",path);

    free(directories);
    return NULL;
}

char * NAME_DEF = "newfile";

// Retorna el padre! Null si no existe un padre. name_child = Null en caso de que ya exista.
struct Inode* searchNew(const char* path, char **name_child){ // Persona 2
    printf("LOOK FOR NEW %s\n", path);
    int count;
    char **directories = split(path, "/", &count);
    char *target = strdup(directories[count - 1]);
    char *parent_path = malloc(strlen(path) + count + NULL_CHAR);
    *parent_path = '\0';

    if (count == 1) {
        parent_path = strcpy(parent_path, ROOT_DIR);
    } else {
        //parent_path[0] = '\0';
        for (int i = 0; i < count - 1; i++) {
            strcat(parent_path, "/");
            strcat(parent_path, directories[i]);
        }
    }

    free(directories);

    struct Inode* parent = searchRelative(parent_path);

    free(parent_path);

    if (!parent) {
        printf("NO PARENT FOR %s\n", path);
        free(target);
        return NULL;
    }

    struct Inode* child = searchChild(parent, target);

    if (child) {
        printf("GOT NAME FOR %s %s\n", target, child->name);
        free(target);
        return parent;
    }

    *name_child = strdup(target);
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

