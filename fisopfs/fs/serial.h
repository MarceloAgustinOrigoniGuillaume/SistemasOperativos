#ifndef SERIAL_H
#define SERIAL_H

struct SerialFD {
    int fd;
    int wrote_count;
    //Writer(int _fd, int _wrote): fd(_fd), wrote_count(_wrote){} 
    //Writer(int _fd): {} 
};

int writeInt(struct SerialFD* writer, int num);

int writeShort(struct SerialFD* writer, short num);

// 0 terminated! se escribe directamente y se busca por el 0.
// O bue.. deberia. Por ahora es lo mismo ! que el msg.
int writeStr(struct SerialFD* writer, const char* str);


// len y dsps el buffer
int writeMsg(struct SerialFD* writer, const char* buffer, int count);



int readInt(struct SerialFD* writer, int* num);
int readShort(struct SerialFD* writer, short* num);
char* readStr(struct SerialFD* writer, int * ret);
char * readMsg(struct SerialFD* writer, int* ret);
int readCapMsg(struct SerialFD* writer, char* buffer,short * msg_len, int max);


// Abre el writer abriendo el fd.
struct SerialFD openWriter(const char * filepath, int * ret);
struct SerialFD openReader(const char * filepath, int * ret);


void closeWriter(struct SerialFD* writer);

// "abre" un sub item writer.
// Que consta en reservar un espacio para la length y dup
struct SerialFD openSubWriter(struct SerialFD* writerBase);

// "cierra"/ agrega al writer base el sub item. Y cierra el writer sub itm 
int closeSubWriter(struct SerialFD* writerBase,struct SerialFD* writerSubitm);


#endif
