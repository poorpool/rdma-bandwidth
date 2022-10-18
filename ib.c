#include "ib.h"
#include <stdio.h>

void printQPInfo(const char *hint, struct QPInfo *qwq) {
    printf("%s\n", hint);
    printf("lid %d\n", qwq->lid);
    printf("qpNum %d\n", qwq->qpNum);
    printf("gidIndex %d\n", qwq->gidIndex);
}