#ifndef DATA_H
#define DATA_H

#include "./block.h"
struct Data {
   int size;
}


struct FileData{ // Persona 3
   int size;
   Block* first_block;
}

struct DirData{ // Persona 2
   int size;
   int capacity;
}




#endif
