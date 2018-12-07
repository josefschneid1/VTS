#include "math.h"
#include <rpc/rpc.h>
#include <stdlib.h>

int retValue;

int* add_1_svc(intpair* pair, struct svc_req* rqstp)
{
    
    retValue = pair->a + pair->b;
    return &retValue;
}

int* multiply_1_svc(intpair* pair, struct svc_req* rqstp)
{
    retValue = pair->a * pair->b;
    return &retValue;
}

int* cube_1_svc(int * i, struct svc_req * rqstp)
{
    int number = *i;
    retValue = number * number * number;
    return &retValue;     
}
