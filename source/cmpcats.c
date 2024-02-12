#define _GNU_SOURCE
#include "cmpcats.h"
#include "diff.h"
int main(int argc, char** argv) {
//    printf("%s %s\n",argv[1],argv[2]);
//    printf("%d %d\n", strlen(argv[1]),strlen(argv[2]));
//    return 0;
    char *dirA=NULL,*dirB=NULL,*dirC=NULL;
    int c;
    while ((c = getopt(argc,argv,":s:"))!=-1){
        switch (c) {
            case 's':
                dirC = optarg;
            break;
            case ':':
                dirC = NULL;
                break;
            case '?':
                fprintf(stderr, "invalid option: -%c\n", optopt);
                return EXIT_FAILURE;
        }
    }
    if (optind<argc){
        dirA = argv [optind];
        dirB = argv[optind+1];
    }
//    printf("%s %s %s\n",dirA,dirB,dirC);
//    return 0;
    listPtr* mergeLists = diff(dirA,dirB);
    if (dirC!=NULL){
        merge(dirC,mergeLists);
    }
    for (int i = 0; mergeLists[i]!=NULL; i++) {
        listDstr(mergeLists[i]);
    }
    free(mergeLists);
    //free tis listes
    return 0;
}