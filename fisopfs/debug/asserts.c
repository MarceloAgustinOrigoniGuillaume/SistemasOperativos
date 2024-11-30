#include "./asserts.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct TestRef{
    test_t test;
};

static int count_tests;
static int max_tests;
static struct TestRef* tests;

int testCount(){
    return count_tests;
}

void addTest(test_t test){
    
    if(count_tests >= max_tests){
        printf("Cannot allocate more tests! no capacity!\n");
        return;
    }
    
    (tests+count_tests)->test = test;
    count_tests++;
}



int runTests(bool failFast){
    int ind = 0;
    int count_failed = 0;
    while(ind < count_tests){
        
        test_t test = (tests+ind)->test;
        
        int ret = test();
        
        if(ret != 0){
            printf("----Failed test! %d\n",ind);
            if(failFast){
                return 1;
            }
            
            count_failed++;
            
        } else{
            printf("----PASSED test! %d\n",ind);
        
        }
        
        ind++;
    }
    
    
    return count_failed;
}


int allocTests(int count){
    max_tests = count;
    tests = (struct TestRef *) malloc(sizeof(struct TestRef) * count);
    count_tests = 0;
    return tests? 0: -1;
}
void freeTests(){
    if(tests){
        free(tests);
    }
}

