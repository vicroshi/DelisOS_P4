#define _GNU_SOURCE
#include "cmpcats.h"
#include "diff.h"
int main(int argc, char** argv) {
    listPtr* mergeLists = diff(argv[1],argv[2]);
    merge(argv[3],mergeLists[0],mergeLists[1],mergeLists[2]);
    listDstr(mergeLists[0]);
    listDstr(mergeLists[1]);
    listDstr(mergeLists[2]);
    free(mergeLists);
    //free tis listes
    return 0;
}