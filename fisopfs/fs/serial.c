#include "./serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define FAIL_FAST 1

void showBytes(const uint8_t* buff, int count){
    printf("count: %d 0x%02X(%d)",count, *buff,(char)*buff);
    
    for(int ind = 1; ind< count; ind++){
        uint8_t vl = *(buff+ind);
        printf(", %d: 0x%02X(%d)",ind,vl, (char)vl);
    }
    printf("\n");
    
}


static int tryReadAll(int fd, uint8_t* out, int count){
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

static int tryWriteAll(int fd, const uint8_t* src, int count){
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

static int writeAll(struct SerialFD* writer, const uint8_t* src, int count){
     int wr = tryWriteAll(writer->fd, src, count);
     if(wr != count){
         fprintf(stderr,"FAILED WRITE! only wrote %d of %d\n",wr,count);
         
         #if FAIL_FAST == 1
         fprintf(stderr,"FAIL FAST\n");
         exit(errno);
         #endif
         
         return errno;
     }
     writer->wrote_count+= count;
     
     //printf(" WROTE:");
     //showBytes(src, count);
     
     return 0;
}

static int readAll(int fd, uint8_t* out, int count){
     int rd = tryReadAll(fd, out, count);
     if(rd != count){
         fprintf(stderr,"FAILED READ! only read %d of %d\n",rd,count);
         #if FAIL_FAST == 1
         fprintf(stderr,"FAIL FAST\n");
         exit(errno);
         #endif

         return errno;
     }     
     //printf(" READ:");
     //showBytes(out, count);
     
     return 0;
}



int writeInt(struct SerialFD* writer, int num){
    num = htonl(num);
    //printf("WROTE INT BIG %d\n", num);
    return writeAll(writer, (uint8_t*)&num, sizeof(int));
}

int readInt(struct SerialFD* writer, int* num){
    int ret= readAll(writer->fd, (uint8_t*)num, sizeof(int));
    if(ret ==0){
        //printf("READED INT BIG %d\n", *num);
        *num = ntohl(*num);
    }
    
    return ret;
}

int writeShort(struct SerialFD* writer, short num){
    num = htons(num);
    return writeAll(writer, (uint8_t*)&num, sizeof(short));
}

int readShort(struct SerialFD* writer, short* num){
    int ret= readAll(writer->fd, (uint8_t*)num, sizeof(short));
    if(ret ==0){
        //printf("READ SHORT %d\n", *num);
        *num = ntohs(*num);
    }
    
    return ret;
}


// len y dsps el buffer
int writeMsg(struct SerialFD* writer, const char* buffer, int count){
    int err = writeShort(writer, count);
    if(err != 0){
        return err;
    }
    
    return writeAll(writer, (const uint8_t*)buffer , count);
}
// 0 terminated! se escribe directamente y se busca por el 0.
int writeStr(struct SerialFD* writer, const char* str){
    return writeMsg(writer, str, strlen(str));
}


int readStr(struct SerialFD* writer, char** out){
     short msg_len;
     int ret = readShort(writer, &msg_len);
     if(ret != 0){
         return ret;
     }
     
     //printf("---> READ STRLEN! %d \n",msg_len);
     
     uint8_t * res = (uint8_t*) malloc(sizeof(uint8_t)*(msg_len+1)); 
     if(res == NULL){
         #if FAIL_FAST == 1
         fprintf(stderr,"FAIL FAST FAILED ALLOC STR\n");
         exit(-1);
         #endif     
         return -1;
     }
     
     ret = readAll(writer->fd, res, msg_len);
     if(ret != 0){
         free(res);
         return ret;
     }
     // Por las dudas
     *(res+msg_len) = 0;
     *out = (char*)res;
     return 0;
}
char * readMsg(struct SerialFD* writer, int* ret){
     short msg_len;
     *ret = readShort(writer, &msg_len);
     if(*ret != 0){
         return NULL;
     }
     
     //printf("READ MSG len %d\n", msg_len);
     uint8_t * res = (uint8_t*) malloc(sizeof(uint8_t)*(msg_len));  
     if(res == NULL){
         #if FAIL_FAST == 1
         fprintf(stderr,"FAIL FAST FAILED ALLOC STR\n");
         exit(-1);
         #endif      
         *ret = -1;
         return NULL;
     }
     *ret = readAll(writer->fd, res, msg_len);
     if(*ret != 0){
         free(res);
         return NULL;
     }
     return (char*)res;
}

int readCapMsg(struct SerialFD* writer, char* buffer, short * msg_len, int max){
     int ret = readShort(writer, msg_len);
     if(ret != 0){
         return ret;
     }
     
     if(*msg_len > max){
         fprintf(stderr, "Went out of bounds at read msg for buffer!\n");
         #if FAIL_FAST == 1
         fprintf(stderr,"FAIL FAST OUT OF CAP\n");
         exit(-1);
         #endif          
         return -1;
     }
     
     return readAll(writer->fd, (uint8_t*)buffer, *msg_len);
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

