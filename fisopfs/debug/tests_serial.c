#include "./asserts.h"
#include "./tests_serial.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../fs/serial.h"

static char* filename = NULL; //= "tmp_serial";
void setPath( char * file){
    filename = file;
    printf("SET SERIAL TEST TARGET FILE %s\n", file);
}


int some_err(){
    return 3;
}
int test_simple_fail(){
    int err;
    
    EXPECT_EQ(2, 1, "deberia ser 1! %d");

    ASSERT_EQ(err,some_err(), 1, "deberia ser 1 ASSERT! %d");

    return 0;

}

int test_simple_success(){
    int err;
    
    EXPECT_EQ(1, 1, "deberia ser 1! %d");

    ASSERT_EQ(err,some_err(), 3, "deberia ser 3 ASSERT! %d");

    return 0;

}




int test_simple(){
     int err = 0;
     struct SerialFD writer = openWriter(filename, &err);
     
     EXPECT_EQ(err, 0, "Hubo error al abrir el writer err %d");
     
     ASSERT_EQ(err, writeInt(&writer, 25), 0 , "Hubo error al escribir el 25,err %d")
     ASSERT_EQ(err, writeStr(&writer, "UNO2"), 0, "Error al escribir UNO,err %d")
     ASSERT_EQ(err, writeInt(&writer, 50), 0 , "Hubo error al escribir el 50,err %d")
     
     closeWriter(&writer);
     
     

     struct SerialFD reader = openReader(filename, &err);
     EXPECT_EQ(err, 0, "Hubo error al abrir el reader err %d");
     
     int out;
     char * outStr = NULL;

     ASSERT_EQ(err, readInt(&reader, &out), 0 , "Hubo error al leer numero, err %d")
     EXPECT_EQ(out, 25, "El valor leido para 25 fue %d")


     ASSERT_EQ(err, readStr(&reader, &outStr), 0 , "Hubo error al leer string, err %d")
     EXPECT_EQ_STR(outStr, "UNO", "El valor leido de string no fue 'UNO' fue %s")


     ASSERT_EQ(err, readInt(&reader, &out), 0 , "Hubo error al leer numero, err %d")
     EXPECT_EQ(out, 50, "El valor leido para 50 fue %d")

     closeWriter(&reader);
     
     return 0;
}

int countSerialTests(){
    return 2;
}

void initSerialTests(){
    
    addTest(&test_simple_success);
    addTest(&test_simple);
    
    //addTest(&test_simple_fail);
}

