#include "./directories.h"
#include "./inodes.h"
#include "./blocks.h"
#include "./serial.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EXIT_FAILURE 1
#define ROOT_DIR "/"
#define NULL_CHAR 1

int cant_dirs = 0;

static char* splitLast(char* temp, const char* delimiter, int *ind) {
    int length = strlen(temp);
    
    char * prev = strtok(temp, delimiter);
    char * last = prev;
    
    *ind = -1;
    
    while (last != NULL) {
        printf("FOUND SEG '%s'\n", last);
        prev = last;
        last = strtok(NULL, delimiter);
    }
    
    if(prev){
        printf("FINAL RES IS '%s'\n",prev);
        
        *ind = length-strlen(prev);
        
    }
    
    return prev;
}

static struct DirData * first_free;
static int new_dir_id; 


static struct DirData * resetDirData(int id){
    struct DirData * dir = &dirarr[id];
    
    dir->id_dir = id;
    dir->size = 0; // 0 children!
    dir->next_free = NULL;
    
    dir->capacity = INIT_DIR_ENTRIES;

    // Por suerte ya estan en code segment,... no en el heap!
    for (int i = 0; i < dir->capacity; i++) {
        dir->entries_id[i] = NOT_DEFINED_BLOCK;
    }
    
    return dir;
}

static struct DirData * getDirData(int id){
    if(id < 0 || id >= new_dir_id){
         return NULL;
    }
    
    return &dirarr[id];
}

static void freeChildrenData(struct DirData * parent){
    if(parent == NULL){
         return;
    }
    
    for (int i = 0; i < parent->size; i++) {
        struct Inode * child = getinode(parent->entries_id[i]);
        if(child == NULL){
            continue; // Por las dudas!
        }
        
        if(child->type == I_DIR){
             freeDir(child);
        } else{
             freeFile(child);
        }
        deleteInode(child); // Al ser recursivo hace falta!
    }
}



static void freeDirData(int id){
    struct DirData * prev_free = first_free;
    struct DirData * next_free = first_free->next_free;
    
    while(next_free){
        
        if(prev_free->id_dir < id && next_free->id_dir > id){
              //Add in between
              struct DirData * curr = resetDirData(id);
              prev_free->next_free = curr;
              curr->next_free = next_free;
              cant_dirs--; // Reduzco la cantidad de dirs
              return;
        }
        
        next_free = next_free->next_free;
    }
    
    // Se llego al final? por lo que se debe agregar al final como nuevo free
    prev_free->next_free = resetDirData(id); // Agrega a la lista de frees el bloque curr
}

static int getFreeDirData(){
    int id = first_free->id_dir;
    
    first_free->size = 0;
    
    if(first_free->next_free == NULL){
         // Check de capacidad?
         first_free->next_free = resetDirData(new_dir_id++);
    }
    
    first_free = first_free->next_free; // Pop
    
    return id;
}

// Libera al directorio, recursivamente
void freeDir(struct Inode* dir){ // Persona 2
    printf("FREEING DIR REC %s blcok: %d\n", dir->name, dir->first_block);
    
    struct DirData * data = getDirData(dir->first_block);
    if(data){
        freeChildrenData(data); // Recursive free!
        freeDirData(data->id_dir);
    }    
    
    
}


void initDirs(){
    first_free = resetDirData(0);
    new_dir_id = 1;
}

void serializeDirData(struct SerialFD* fd_out, struct DirData*  dir){ 
    printf("SERIALIZE DIR DATA %d\n", dir->id_dir);
    writeInt(fd_out, dir->id_dir);
    writeInt(fd_out, dir->size);
    writeInt(fd_out, dir->capacity);
    
    for (int j = 0; j < dir->capacity; j++) {
        writeInt(fd_out, dir->entries_id[j]);
    }
}
void deserializeDirData(struct SerialFD* fd_in, struct DirData*  dir){ 
    printf("DESERIALIZE DIR DATA %d\n", dir->id_dir);
    readInt(fd_in, &(dir->size));
    readInt(fd_in, &(dir->capacity));
    
    printf("DIR DATA %d cap: %d\n", dir->size, dir->capacity);
    for (int j = 0; j < dir->capacity; j++) {
        readInt(fd_in, &(dir->entries_id[j]));
        if(dir->entries_id[j]>=0){
            printf("CHILD %d \n", dir->entries_id[j]);
        }
    }
}


void serializeDirs(struct SerialFD* fd_out){ 
    // No se usa el fd directamente! por tema little endian vs big endian y asi
    // Para numeros y asi esta los metodos de serial.h!
    printf("Serialize dirs data.. %d size dirs: %d\n",fd_out->fd, new_dir_id);
    writeInt(fd_out, cant_dirs);
    printf("CANT DIRS %d\n", cant_dirs);
    
    int left = cant_dirs;
    struct DirData*  next_free = first_free;
    int i = 0;
    
    while(next_free && left >0){
        if(i == next_free->id_dir){ //skip free ones
            next_free = next_free->next_free;
            i++;
            continue;
        }
        serializeDirData(fd_out, &dirarr[i]);
        i++;
        left--;
    }
    
    while(left >0){
        serializeDirData(fd_out, &dirarr[i]);
        i++;
        left--;    
    }
}

void deserializeDirs(struct SerialFD* fd_in){
    printf("Deserialize dirs data.. %d\n",fd_in->fd);

    //new_dir_id = 0;
    int res = readInt(fd_in, &cant_dirs); 
    printf("CANT DIRS %d\n", cant_dirs);

    int last_id = 0;
    first_free = resetDirData(0);
    struct DirData * curr_free = first_free;
    
    for (int i = 0; i < cant_dirs; i++) {
        int id = 0;        
        res = readInt(fd_in, &id);
        if (res == -1) {
            printf("FAILED READ OF DIR DATA ID!");
            return;
        }
        // Deserialize
        
        deserializeDirData(fd_in, resetDirData(id));
        
        while(++last_id < id){ // Add as first!
            curr_free->next_free = resetDirData(last_id);
            curr_free = curr_free->next_free;
            //last_id++;
        }
                
    }
    
    new_dir_id = last_id+1;
    first_free = first_free->next_free;
    if(first_free == NULL){
        first_free= resetDirData(new_dir_id++);         
    }
    
    // Pop first free que era un placeholder.
}


static struct Inode* indexChild(struct DirData* dir,int *ind, const char* name){
    printf("INDEx CHILD: LOOK FOR %s IN %d\n", name, dir->size);
    *ind = NOT_DEFINED_BLOCK;
    for (int i = 0; i < dir->capacity; i++) {
        if(dir->entries_id[i] == NOT_DEFINED_BLOCK){
            continue;
        }
        
        struct Inode * child = getinode(dir->entries_id[i]);
        
        if(child && strcmp(child->name, name) == 0){
            printf("FOUND %s\n", name);
            *ind = i;
            return child;
        }
    }    
    return NULL;
}


char * strndup(const char *src, int count) {
    size_t len = count + 1;
    char *dst = malloc(len);
    if (dst) {
        memcpy(dst, src, len);
        *(dst+len) =0;
    }
    return dst;
}


char *strdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = malloc(len);
    if (dst) {
        memcpy(dst, src, len);
    }
    return dst;
}

int allocDir(struct Inode* dir){ // Persona 2
    printf("ALLOC DIR %s\n", dir->name);
    dir->blocks = 1;
    dir->size_bytes = 4096;// por default capaz?
    dir->first_block = getFreeDirData();
    cant_dirs++; // Aumento la cantidad de dirs
    return 0;
}



// Busca al inodo, y lo remueve del padre. Retorna el inodo hijo. Para su posterior liberacion de hacer falta.
struct Inode* rmChild(const char* path){ // Persona 2
    printf("RM CHILD root %s \n", path);


    char * parent_path = strdup(path);
    int ind = 0;
    char* last_delim = splitLast(parent_path , "/", &ind);
    if(last_delim == NULL || strlen(last_delim) == 1){
        printf("No delim found/invalid name\n");
        free(parent_path);
        return NULL;
    }
    
    char * childname = strdup(last_delim);
    
    free(parent_path);
    parent_path = strndup(path, ind-1);
    
    printf("Parent is '%s' child '%s'\n", parent_path, childname);
    
    struct Inode* parent = searchRelative(parent_path);
    if(parent == NULL){
        printf("Not found parent! '%s'\n", parent_path);
        return NULL;
    }
    
    struct DirData* data = getDirData(parent->first_block);
    
    int index;
    struct Inode* child = indexChild(data, &index, childname);
    
    if (!child) {
        printf("NO CHILD FOUND ON PARENT %s child '%s'\n", parent_path, childname);
        free(childname);
        free(parent_path);
        return NULL;
    }
    
    
    data->entries_id[index] = NOT_DEFINED_BLOCK; // RESET
    data->size--;
    
    free(childname);
    free(parent_path);

    return child;
}

// Agrega el inodo hijo como hijo al padre.
int addChild(struct Inode* parent, struct Inode* child) { // Persona 2
    printf("ADD CHILD parent %s child: %s\n", parent->name, child->name);

    struct DirData* data = getDirData(parent->first_block);    
    
    if (data->size == data->capacity) {
        perror("No hay espacio para agregar un nuevo hijo");
        return -1;
    }
    for (int i = 0; i < data->capacity; i++) {
        if(data->entries_id[i] == NOT_DEFINED_BLOCK){
            data->entries_id[i] = child->id;
            data->size++;
            printf("SE AGREGO CHILD EN %d id is '%d'\n",i, child->id);
            return 0;
        }
    }
    printf("NO SE AGREGO CHILD \n");
    
    return -1;
}



struct Inode* searchChild(struct DirData* dir, const char* name){
    printf("SEARCH CHILD: LOOK FOR %s IN %d childs\n", name, dir->size);
    
    for (int i = 0; i < dir->capacity; i++) {
        if(dir->entries_id[i] == NOT_DEFINED_BLOCK){
            continue;
        }
    
        struct Inode * child = getinode(dir->entries_id[i]);
        
        if(child && strcmp(child->name, name) == 0){
            printf("-->FOUND '%s'\n", name);
            return child;
        } else{
           if(child){
              printf("VALID ID?! %d '%s' vs '%s'\n",dir->entries_id[i], child->name, name);
           } else{
              printf("INVALID ID?! %d\n",dir->entries_id[i]);
           }
        }
    }    
    return NULL;
}

struct Inode* searchRelative(const char* path){ // Persona 2
    struct Inode* root = getinode(0); // ROOT
    printf("SEARCH RELATIVE: LOOK FOR %s ... root is '%s' %d\n", path, root->name, root->first_block);

    if(strlen(path) == 0 || strcmp(path, root->name) == 0){
        return root;
    }

    int count;
    char **directories = split(path, "/", &count);
    char ** target = &directories[0];

    printf("COUNT %d\n", count);
    printf("FIRST TARGET '%s'\n", *target);
    
    struct Inode* child = searchChild(getDirData(root->first_block), *target);
    count--;
    while(child && count > 0){
         if(child->type != I_DIR){
              printf("Segment that was a parent, was not a dir!\n");
              free(directories);
              return NULL;
         }
         
         count--;
         target = target+1; // Anda al siguiente puntero.         
         child = searchChild(getDirData(child->first_block), *target);
    }
    
    free(directories);
    return child;
}

// Retorna el padre! Null si no existe un padre. name_child = Null en caso de que ya exista.
struct Inode* searchNew(const char* path, char **name_child){ // Persona 2
    printf("LOOK FOR NEW '%s'\n", path);

    char * parent_path = strdup(path);
    int ind = -1;
    char* last_delim = splitLast(parent_path , "/", &ind);
    if(last_delim == NULL || strlen(last_delim) == 1){
        printf("No delim found/invalid name\n");
        free(parent_path);
        return NULL;
    }
    
    char * childname = strdup(last_delim);
    printf("Parent bfr is '%s' child '%s' %d\n", parent_path, childname,ind);
    
    free(parent_path);
    parent_path = strndup(path, ind-1);
    
    printf("Parent is '%s' child '%s'\n", parent_path, childname);
    
    //char **directories = split(path, "/", &count);
    //char *parent_path = malloc(strlen(path) + 1);
    //for (int i = 0; i < count - 1; i++) {
    //    strcat(parent_path, "/");
    //    strcat(parent_path, directories[i]);
    //}
    
    struct Inode* parent = searchRelative(parent_path);
    
    if(parent == NULL){
        printf("NOT FOUND PARENT FOR NEW! %s\n", parent_path);
        free(childname);
        free(parent_path);
        
        return NULL;    
    }
    
    struct DirData* data = getDirData(parent->first_block);
    struct Inode* child = searchChild(data, childname);
    if(child){
        *name_child = NULL;
        printf("ALREADY EXISTED ON %s '%s'\n", parent_path, childname);
        free(childname);
        free(parent_path);
        
        return parent;
    }
    
    *name_child = strdup(childname);
    
    free(childname);
    free(parent_path);
    
    return parent;
}

void readChildren(struct Inode* dir, struct DirEntries* out){ // Persona 2
    
    struct DirData* data = getDirData(dir->first_block);
    if(data == NULL){
        printf("---->DIR DATA INVALID AT READ DIR?!\n");
        out->size = 0;
        return;
    }
    printf("---->DIR DATA SIZE %d cap: %d\n",data->size,data->capacity);
    out->size = data->size;
    out->first = (inode_id_t*) malloc(sizeof(inode_id_t) * data->size);
    
    inode_id_t* curr = out->first;
    
    for (int i = 0; i < data->capacity; i++) {
        if(data->entries_id[i]< 0){
             continue;
        }
        printf("---->VALID CHILD %d: \n",data->entries_id[i]);
        
        *curr = data->entries_id[i];
        curr = curr+1; // Next! 
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

