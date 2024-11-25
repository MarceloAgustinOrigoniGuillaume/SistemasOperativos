#define FUSE_USE_VERSION 30

#include "fs/serial.h"
#include <stdio.h>
#include <stdlib.h>



static int writeData(const char * filename){
     printf("ALL GOOD?! %s\n",filename);
     int err = 0;
     struct SerialFD writer = openWriter(filename, &err);
     if(err != 0){
          printf("NOT GOOD!!!%d \n", err);
          return err;
     }
     
     err = writeStr(&writer,"\nSEGUNDO MENSAJE\n");
     if(err != 0){
         printf("FAILED WRITE!%d\n",err);
         return err;
     }
     err = writeInt(&writer, 23);
     if(err != 0){
         printf("FAILED WRITE!%d\n",err);
         return err;
     }
     
     err = writeStr(&writer,"UN MENSAJE Y MAS");
     if(err != 0){
         printf("FAILED WRITE!%d\n",err);
         return err;
     }
     err = writeInt(&writer, 54);
     if(err != 0){
         printf("FAILED WRITE!%d\n",err);
         return err;
     }
     
     printf("YES ALL GOOD!!!\n");
     
     closeWriter(&writer);
     return 0;
}

static int showData(const char * filename){
     printf("ALL GOOD?! %s\n",filename);
     int err = 0;
     struct SerialFD reader = openReader(filename, &err);
     if(err != 0){
          printf("NOT GOOD!!!%d \n", err);
          return err;
     }
     
     char * first = readStr(&reader, &err);
     printf("FIRST '%s' \n", first);
     if(err != 0){
         printf("FAILED READ!%d\n",err);
         return err;
     }
     int res = 2;
     err = readInt(&reader, &res);
     if(err != 0){
         printf("FAILED READ!%d\n",err);
         return err;
     }
     printf("GOT NUM %d\n",res);

     char * second = readStr(&reader, &err);
     printf("SECOND '%s'\n", second);
     
     
     
     err = readInt(&reader, &res);
     if(err != 0){
         printf("FAILED READ!%d\n",err);
         return err;
     }
     printf("GOT NUM %d\n",res);
     
     free(first);
     free(second);
     
     closeWriter(&reader);
     
     return 0;
}



int
main(int argc, char *argv[])
{
     int ret = writeData(argv[1]);
     if(ret != 0){
         return ret;
     }
     
     return showData(argv[1]);
}
