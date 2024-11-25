#include "./serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>


static int tryReadAll(int fd, char* out, int count){
     //printf("TRY READ %d\n", count);
     int rd = read(fd, out , count);
     //printf("RD %d\n", rd);
     int tot = 0;
     while(rd >0 && count > rd){ // Si count == rd ya se leyo todo!
         tot+= rd;
         count -= rd;
         rd = read(fd, out+tot , count);
         //printf("RD NEXT at +%d count:%d rd: %d\n", tot, count,rd);
     }
     
     //printf("LST RD at +%d count:%d rd : %d\n", tot, count,rd);
     // Ultimo read!
     if(rd > 0){
         return tot+rd;
     }
     return tot;
}

static int tryWriteAll(int fd, const char* src, int count){
     int wr = write(fd, src , count);
     int tot = 0;
     
     while(wr >0 && count > wr){
         tot+= wr;
         count -= wr;
         wr = write(fd, src+tot , count);
     }
     // Ultimo write!
     if(wr > 0){
         return tot+ wr;
     }
     
     return tot;
}

static int writeAll(struct SerialFD* writer, const char* src, int count){
     if(tryWriteAll(writer->fd, src, count) != count){
         writer->wrote_count+= count;
         return errno;
     }
     
     return 0;
}

static int readAll(int fd, char* out, int count){
     if(tryReadAll(fd, out, count) != count){
         return errno;
     }     
     return 0;
}



int writeInt(struct SerialFD* writer, int num){
    if(write(writer->fd,&num,1) == -1){
        return errno;
    }
    writer->wrote_count+= sizeof(int); 
    return 0;
}

int readInt(struct SerialFD* writer, int* num){
    if(read(writer->fd,num,1) == -1){
        return errno;
    }
    return 0;
}

int writeShort(struct SerialFD* writer, short num){
    if(write(writer->fd,&num,1) == -1){
        return errno;
    }
    writer->wrote_count+= sizeof(short); 
    return 0;

}

int readShort(struct SerialFD* writer, short* num){
    if(read(writer->fd,num,1) == -1){
        return errno;
    }
    return 0;
    //return readAll(writer->fd, (char*)&num , sizeof(short));
}


// len y dsps el buffer
int writeMsg(struct SerialFD* writer, const char* buffer, int count){
    int err = writeShort(writer, count);
    if(err != 0){
        return err;
    }
    
    return writeAll(writer, buffer , count);
}
// 0 terminated! se escribe directamente y se busca por el 0.
int writeStr(struct SerialFD* writer, const char* str){
    return writeMsg(writer, str, strlen(str));
}


char* readStr(struct SerialFD* writer, int * ret){
     short msg_len;
     *ret = readShort(writer, &msg_len);
     if(*ret != 0){
         return NULL;
     }
     printf("---> READ MSGLEN! %d \n",msg_len);
     
     char * res = (char*) malloc(sizeof(char)*(msg_len+1)); 
     if(res == NULL){
         *ret = -1;
         return NULL;
     }
     *ret = readAll(writer->fd, res, msg_len);
     if(*ret != 0){
         free(res);
         return NULL;
     }
     // Por las dudas
     *(res+msg_len+1) = 0;
     return res;
}
char * readMsg(struct SerialFD* writer, int* ret){
     short msg_len;
     *ret = readShort(writer, &msg_len);
     if(*ret != 0){
         return NULL;
     }
     
     char * res = (char*) malloc(sizeof(char)*(msg_len));  
     if(res == NULL){
         *ret = -1;
         return NULL;
     }
     *ret = readAll(writer->fd, res, msg_len);
     if(*ret != 0){
         free(res);
         return NULL;
     }
     return res;
}

int readCapMsg(struct SerialFD* writer, char* buffer, short * msg_len, int max){
     int ret = readShort(writer, msg_len);
     if(ret != 0){
         return ret;
     }
     
     if(*msg_len > max){
         fprintf(stderr, "Went out of bounds at read msg for buffer!\n");
         return -1;
     }
     
     return readAll(writer->fd, buffer, *msg_len);
}



struct SerialFD openWriter(const char * filepath, int* err){
     int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC,
			            S_IRUSR | S_IWUSR);
     
     if(fd < 0){
         *err = errno;
     }
     
     // fd , wrote_count
     struct SerialFD res;
     res.fd = fd;
     res.wrote_count = 0;
     return res;//{fd, 0};
}

struct SerialFD openReader(const char * filepath, int* err){
     int fd = open(filepath, O_RDONLY);
     
     if(fd < 0){
         *err = errno;
     }
     
     // fd , wrote_count
     struct SerialFD res;
     res.fd = fd;
     res.wrote_count = 0;
     return res;//{fd, 0};
}


void closeWriter(struct SerialFD* writer){
    close(writer->fd);
    writer->fd = -1;
}


// Reserva la cantidad necesaria para el len de un msg.
// En el writer, la reserva y avanza para seguir con los datos
// En el writerLen deja el offset de la posicion para que se guarde a futuro.
// Con el writerMsgLen

#define MSG_LEN_BYTES sizeof(short)
struct SerialFD openSubWriter(struct SerialFD* writerBase){
    lseek(writerBase->fd, MSG_LEN_BYTES, SEEK_CUR); // Reserva espacio.
    writerBase->wrote_count+=MSG_LEN_BYTES;
    struct SerialFD res;
    res.fd = writerBase->fd;
    res.wrote_count = 0;
    
    return res;//Writer(writerBase->fd);
}

// "cierra"/ agrega al writer base el sub item. Y cierra el writer sub itm 
int closeSubWriter(struct SerialFD* writerBase,struct SerialFD* writerSubitm){
    int len = writerSubitm->wrote_count;
    
    // Volve hacia atras en el writer fd, la len y los MSG_LEN_BYTES reservados!
    int ret = lseek(writerBase->fd, -len-MSG_LEN_BYTES, SEEK_CUR);
    if(ret != 0){
        return ret;
    }
    
    ret = writeShort(writerBase, len);
    if(ret != 0){
        return ret;
    }
    ret = lseek(writerBase->fd, len, SEEK_CUR); // Volve al tope/final subitem
    
    if(ret != 0){
        return ret;
    }

    writerSubitm->fd = -1; // Resetea por las dudas.. i.e close.
    return 0;
}

