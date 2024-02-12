#define _GNU_SOURCE
#include "cmpcats.h"
#include "diff.h"
int main(int argc, char** argv) {
//    printf("%s %s\n",argv[1],argv[2]);
//    printf("%d %d\n", strlen(argv[1]),strlen(argv[2]));
//    return 0;
    listPtr* mergeLists = diff(argv[1],argv[2]);
    merge(argv[3],mergeLists);
    for (int i = 0; mergeLists[i]!=NULL; i++) {
        listDstr(mergeLists[i]);
    }
    free(mergeLists);
    //free tis listes
    return 0;
}