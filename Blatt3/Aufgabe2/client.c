#include "math.h"
#include <rpc/rpc.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        printf("Zu wenige Parameter\n");
        exit(1);
    }
    
    char* name_server = argv[1];
    int a = atoi(argv[2]);
    int b = atoi(argv[3]);
        
    struct intpair numbers = {a ,b};


    CLIENT* c1 = clnt_create(name_server, MATHPROG, MATHVERS, "tcp");
    
    printf("Result: %d\n", *add_1(&numbers, c1));


    return 0;
}
