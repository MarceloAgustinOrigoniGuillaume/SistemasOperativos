#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#define INIT_DIR_ENTRIES 128
#define INIT_DIRECTORIES 64

#include "./inode.h"
#include "./serial.h"

struct DirData{ // Persona 2
   int size;
   int capacity;
   int entries_id[INIT_DIR_ENTRIES];
   
   
   int id_dir;
   struct DirData* next_free;
};

struct DirData dirarr[INIT_DIRECTORIES];


void initDirs();


void serializeDirs(struct SerialFD* fd_out);    // Persona 3/1?
void deserializeDirs(struct SerialFD* fd_in);   // Persona 3/1?

#define BLOCK_COUNT 512
// Manejo de directorios

// Para buscar desde cierto inodo directorio, puede servir si se da soporte
// A opendir y cosas asi.
struct Inode* searchRelative(const char* path); // Persona 2

// Busca el padre y retorna el nombre con el cual deberia crearse el nuevo hijo.
// Si ya existe o no se permite. name_child == NULL
struct Inode* searchNew(const char* path, char **name_child); // Persona 2

// Para el ls, lista los hijos en el struct out.
void readChildren(struct Inode* dir, struct DirEntries* out); // Persona 2

// Crea los datos del dir
int allocDir(struct Inode* dir); // Persona 2

// Libera al directorio, recursivamente
void freeDir(struct Inode* dir); // Persona 2

// Busca al inodo, y lo remueve del padre. Retorna el inodo hijo. Para su posterior liberacion de hacer falta.
struct Inode* rmChild(const char* path); // Persona 2

// Agrega el inodo hijo como hijo al padre.
int addChild(struct Inode* parent, struct Inode* child); // Persona 2

char ** split(const char* str, const char* delimiter, int* count);

#endif
