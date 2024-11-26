#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#define INIT_DIR_ENTRIES 256

#include "./inode.h"

struct DirEntries{
    struct Inode* inode; // id del inodo? o puede ser algo interno.
};

struct DirData{ // Persona 2
   int size; // Campo de Inode Data*, tiene que estar arriba de todo!
   int capacity;
struct DirEntries entries[INIT_DIR_ENTRIES];
};

void serializeDirData(int fd_out, const struct DirData* data_out);    // Persona 2/1?
void deserializeDirData(int fd_in, struct DirData* data);   // Persona 2/1?

// Function to initialize DirData
void initDirData(struct DirData* dirData, int initialCapacity);

// Function to free DirData
void freeDirData(struct DirData* dirData);

// Manejo de directorios

// Para buscar desde cierto inodo directorio, puede servir si se da soporte
// A opendir y cosas asi.
struct Inode* searchRelative(const char* path); // Persona 2

// Busca el padre y retorna el nombre con el cual deberia crearse el nuevo hijo.
// Si ya existe o no se permite. name_child == NULL
struct Inode* searchNew(const char* path, char ** name_child); // Persona 2

// Para el ls, lista los hijos en el struct out.
void readChildren(struct Inode* dir, struct DirEntries* out); // Persona 2

// Libera al directorio, recursivamente
void freeDir(struct Inode* dir); // Persona 2

// Busca al inodo, y lo remueve del padre. Retorna el inodo hijo. Para su posterior liberacion de hacer falta.
struct Inode* rmChild(const char* path); // Persona 2

// Agrega el inodo hijo como hijo al padre.
int addChild(struct Inode* parent, struct Inode* child); // Persona 2

char ** split(const char* str, const char* delimiter, int* count);

#endif
