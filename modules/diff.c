//
// Created by vic on 27/01/2024.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
//#include <fts.h>
#include "diff.h"
#include "list.h"
int filter(const struct dirent* dir){
    return dir->d_name[0]!='.';
}

char* construct_path(char* path,char* name){
    char *new_path=malloc(strlen(path)+strlen(name)+2);
    if(new_path==NULL){
        perror("malloc()");
        exit(1);
    }
    strcpy(new_path,path);
    strcat(new_path,"/");
    strcat(new_path,name);
    return new_path;
}

char* construct_relative_path(char *path){//apo to full path, kratame mono to path mesa apo to folder
    char *relative_path;
    int index;
    for(index=0;index < strlen(path);index++) if(path[index++]=='/') break; //mexri to prwto '/' pou tha broume, ++ gia na pame ston xaraktira META to '/'
    relative_path=malloc((strlen(path)-index+1)*sizeof(char));
    if(relative_path==NULL){
        perror("malloc()");
        exit(1);
    }
    memcpy(relative_path,path+index,strlen(path)-index+1); //+1 gia to terminating character
    return relative_path;
}

//dinontas 2 paths arxeiwn, elegxw an exoun idio size kai idia contents
int files_have_same_contents(char *filepathA,char*filepathB,size_t file_size){
    char a,b;
    int fd1,fd2;
    fd1=open(filepathA,O_RDONLY);
    if(fd1==-1){
        perror("open()");
        exit(1);
    }
    fd2=open(filepathB,O_RDONLY);
    if(fd2==-1){
        perror("open()");
        exit(1);
    }

    for(size_t i=0; i<file_size;i++){
        if(read(fd1,&a,1)==-1){
            perror("read()");
            exit(1);
        }
        if(read(fd2,&b,1)==-1){
            perror("read()");
            exit(1);
        }
        if(a!=b){//brikame byte to opoio den einai idio, ara ta arxeia den exoun idia contents
            close(fd1);
            close(fd2);
            return 0;
        }
    }
    close(fd1);
    close(fd2);
    return 1;
}

int compare_links(char* pathA, char* pathB, struct stat* stA, struct stat* stB, listPtr diffA, listPtr diffB){
    char link_pathA[PATH_MAX];
    char link_pathB[PATH_MAX];
    strcpy(link_pathA,pathA);
//                        link_pathB = malloc(strlen(new_pathB)+1);
    strcpy(link_pathB,pathB);
    char flag = 1;
    while (flag){
        char targetA[stA->st_size];
        char* target_pathA;
        char targetB[stA->st_size];
        char* target_pathB;
        int nbytesA,nbytesB;
        char* target_dirA;
        char* target_baseA;
        char* target_dirB;
        char* target_baseB;
        nbytesA = readlink(link_pathA,targetA,stA->st_size+1);
        if (nbytesA==-1){
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        target_dirA = dirname(targetA);
        target_baseA = basename(targetA);
        nbytesB = readlink(link_pathB,targetB,stB->st_size+1);
        if (nbytesB==-1){
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        target_dirB = dirname(targetB);
        target_baseB = basename(targetB);
        if (!strcmp(targetA,targetB)){
            struct stat st_targA,st_targB;
            target_pathA = construct_path(link_pathA,targetA);
            lstat(target_pathA,&st_targA);
            target_pathB = construct_path(link_pathB,targetB);
            lstat(target_pathB,&st_targB);
            if ((st_targA.st_mode & S_IFMT) == (st_targB.st_mode & S_IFMT))
                switch (st_targA.st_mode & S_IFMT) {
                    case S_IFREG:
                        if (!(st_targA.st_size == st_targB.st_size && files_have_same_contents(target_pathA,target_pathB,st_targA.st_size))){
                            listInsert(diffA,target_pathA);
                            listInsert(diffA,target_pathB);
                        }
                        flag = 0;
                        free(target_pathA);
                        free(target_pathB);
//                                        free(link_pathA);
//                                        free(link_pathB);
                        break;
                    case S_IFLNK:
//                                        free(link_pathA);
//                                        link_pathA = malloc(strlen(target_pathA)+1);
                        strcpy(link_pathA,target_pathA);
//                                        free(link_pathB);
//                                        link_pathB = malloc(strlen(target_pathB)+1);
                        strcpy(link_pathB,target_pathB);
                        free(target_pathA);
                        free(target_pathB);
                        continue;
                    default:
                        break;
                }
        }
//                            if(targetA[0] !=
//                            '/'){
//
//                            }
    }
}

void compare(char* pathA, char* pathB,listPtr diffA,listPtr diffB){
    int lenA = 0,lenB = 0;
    struct dirent** a = NULL;
    struct dirent** b = NULL;
    if (pathA){
        lenA = scandir(pathA,&a,filter,alphasort);
    }
    if (pathB){
        lenB = scandir(pathB,&b,filter,alphasort);
    }
    int i = 0, j = 0;
    char* new_pathA = NULL,*new_pathB = NULL;
    struct stat stA,stB;
    while(i<lenA && j<lenB){
        if(strcmp(a[i]->d_name,b[j]->d_name)<0){
            //lenA--;  //allagi sunthikis, to sbinw gia na kserw megethos pinaka gia free meta
            new_pathA = construct_path(pathA,a[i++]->d_name);
            listInsert(diffA,new_pathA);
            lstat(new_pathA,&stA);
            if( (stA.st_mode&S_IFMT) ==S_IFDIR){ //an einai dir, prepei na mpoume na to elegksoume
                compare(new_pathA, NULL,diffA,NULL);
            }
            free(new_pathA);
        }
        else if(strcmp(a[i]->d_name,b[j]->d_name)>0){
           // lenB--;
            new_pathB =  construct_path(pathB,b[j++]->d_name);
            listInsert(diffB, new_pathB);
            lstat(new_pathB,&stB);
            if ((stB.st_mode&S_IFMT )== S_IFDIR){ //an einai dir, prepei na mpoume na to elegksoume
                compare(NULL,new_pathB,NULL,diffB);
            }
            free(new_pathB);
        }
        else{  //ta arxeia exoun to idio onoma, elegxw an exoun kai idio tupo. an diaferoun se kati, mpainoun kai stis duo listes
           // lenA--;
           // lenB--;
            new_pathA = construct_path(pathA,a[i++]->d_name);
            new_pathB = construct_path(pathB,b[j++]->d_name);
            lstat(new_pathA,&stA);
            lstat(new_pathB,&stB);

            if ((stA.st_mode&S_IFMT) == (stB.st_mode&S_IFMT)){
                char flag = 1;
                char link_pathA[PATH_MAX];
                char link_pathB[PATH_MAX];
                switch (stA.st_mode&S_IFMT) {
                    case S_IFDIR:
                        compare(new_pathA,new_pathB,diffA,diffB);
                        break;
                    case S_IFREG: //exoun idio name kai eiani regular files. elegxw an exoun idio size kai an ta contents einai ta idia
                        //compare files
                        if( !(stA.st_size==stB.st_size && (files_have_same_contents(new_pathA,new_pathB,stA.st_size))) ){ //an den exoun idio size kai den exoun idia contents, den einai to idio arxeio, prepei na ta emfanisoume
                            listInsert(diffA,new_pathA);
                            listInsert(diffB,new_pathB);
                        }
                        break;
                    case S_IFLNK:
                        //compare links
//                        link_pathA = malloc(strlen(new_pathA)+1);

                        break;
                    default:
                        break;
                }
            }
            free(new_pathA);
            free(new_pathB);
        }
    }
    for (; i<lenA; i++) {
        new_pathA = construct_path(pathA,a[i]->d_name);
        listInsert(diffA,new_pathA);
        lstat(new_pathA,&stA);
        if ((stA.st_mode&S_IFMT )== S_IFDIR){
            compare(new_pathA,NULL,diffA,NULL);
        }
        free(new_pathA);
//        (*diffA)[ii] = malloc(a[i]->d_reclen);
//        memcpy((*diffA)[ii++],a[i],a[i]->d_reclen);
    }
    for (; j<lenB; j++) {
        new_pathB =  construct_path(pathB,b[j]->d_name);
        listInsert(diffB, new_pathB);
        lstat(new_pathB,&stB);
        if ((stB.st_mode&S_IFMT )== S_IFDIR){
            compare(NULL,new_pathB,NULL,diffB);
        }
        free(new_pathB);
    }
    for(int i=0;i<lenA;i++) free(a[i]);
    free(a);
    for(int j=0;j<lenB;j++) free(b[j]);
    free(b);
}

char diff(char* dirA, char* dirB){
    //dirent api

    //kwdikas pou den xrisimopoieitai
   /* struct dirent** entriesA;
    struct dirent** entriesB;
    int lenB = scandir(dirB,&entriesB,filter,alphasort);
    int lenA = scandir(dirA,&entriesA,filter,alphasort);
    struct dirent** intersection;
//    struct dirent** diffA;
//    struct dirent** diffB;
    int lenIntr;
    int lenDiffA;
    int lenDiffB;
//    cmp_ent(dirA,dirB,entriesA,entriesB,lenA,lenB,
//            &intersection,&diffA,&diffB,&lenIntr,&lenDiffA,&lenDiffB);
//    print_and_free(entriesA,lenA,"dirA");
//    print_and_free(entriesB,lenB,"dirB");
//    print_and_free(diffA,lenDiffA,"diffA");
//    print_and_free(diffB,lenDiffB,"diffB");
//    print_and_free(intersection,lenIntr,"intersection"); */


    listPtr diffA,diffB;
    diffA = listInit(dirA);
    diffB = listInit(dirB);
    compare(dirA,dirB,diffA,diffB);
    printf("In %s:\n",dirA);
    listPrint(diffA);
    printf("\nIn %s:\n",dirB);
    listPrint(diffB);
    listDstr(diffA);
    listDstr(diffB);




    //fts api
    /*char* fts_arg[] = {
            dirA,
            dirB,
            NULL
    };
    FTS* ftsA = fts_open(fts_arg,FTS_PHYSICAL,NULL);
    FTSENT* e;
    FTSENT* entriesA;
    FTSENT* entryA;
    while ((entryA = fts_read(ftsA))){
        entriesA = fts_children(ftsA,0);
        if (entriesA){
            printf("folder:%s\n",entryA->fts_name);
            print_fts(entriesA);
        }
    }*/
    return 1;
}



//#################################################################################################################################################
//#################################################################################################################################################
///synartiseis apo proigoumeno version







/*int cmp_dir(char* pathA, char* pathB *//**//*){
    struct dirent** entriesA;
    struct dirent** entriesB;
    int lenA = scandir(pathA,entriesA,filter,alphasort);
    int lenB = scandir(pathB,entriesB,filter,alphasort);

}*/


//void cmp_ent(char* pathA, char* pathB, struct dirent** a, struct dirent**b, int a_len, int b_len, struct dirent*** intersection,struct dirent*** diffA,struct dirent*** diffB,int* i_len, int* dA_len, int* dB_len){
//    int max = (a_len > b_len ? a_len : b_len);
//    *intersection = malloc(sizeof(struct dirent*) * max);
//    *diffA = malloc(sizeof(struct dirent*) * a_len);
//    *diffB = malloc(sizeof(struct dirent*) * b_len);
//    int i = 0, j = 0, k = 0, ii = 0, jj = 0;
//    struct stat stA,stB;
//    char* cwd;
//    while (i < a_len && j < b_len){
////        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
//        if(strcmp(a[i]->d_name,b[j]->d_name)<0){
//            (*diffA)[ii] = malloc(a[i]->d_reclen);
//            memcpy((*diffA)[ii++],a[i],a[i]->d_reclen);
//            i++;
//        }
//        else if(strcmp(a[i]->d_name,b[j]->d_name)>0){
//            (*diffB)[jj] = malloc(b[i]->d_reclen);
//            memcpy((*diffB)[jj++],b[j],b[j]->d_reclen);
//            j++;
//        }
//        else{
//            /*
//             * stat gia elegxo tipou kai inode klp
//             * switch gia to type
//             * kai meta compares
//             */
//            cwd = get_current_dir_name();
//            chdir(pathA);
//            lstat(a[i]->d_name,&stA);
//            printf("%s %ld\n",a[i]->d_name,stA.st_ino);
//            chdir(cwd);
//            chdir(pathB);
//            lstat(b[j]->d_name,&stB);
//            printf("%s %ld\n",b[j]->d_name,stB.st_ino);
//            mode_t typeA = stA.st_mode & S_IFMT;
//            mode_t typeB = stB.st_mode & S_IFMT;
//            if (typeA == typeB){
//                switch (typeA) {
//                    case S_IFDIR:
//                        //cmpdir
//                        break;
//                    case S_IFREG:
//                        //cmpfile
//                        break;
//                    case S_IFLNK:
//                        //cmplink
//                        break;
//                    default:
//                        break;
//                }
//            }
//            else{
//                //einai diaforetika valta sto diff
//            }
//
////            lstat()
////            lstat()
//            (*intersection)[k] = malloc(a[i]->d_reclen);
//            memcpy((*intersection)[k++],a[i],a[i]->d_reclen);
//            i++;
//            j++;
//        }
//    }
//    for (; i < a_len; i++) {
////        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
//        (*diffA)[ii] = malloc(a[i]->d_reclen);
//        memcpy((*diffA)[ii++],a[i],a[i]->d_reclen);
//    }
//    for (; j < b_len; j++) {
////        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
//        (*diffB)[jj] = malloc(b[j]->d_reclen);
//        memcpy((*diffB)[jj++],b[j],b[j]->d_reclen);
//    }
//    //isws kai na mh xreiazetai kan den glittwneis xwros
////    *diffA = realloc(*diffA,ii*sizeof(struct dirent*));
////    *diffB = realloc(*diffB,jj*sizeof(struct dirent*));
////    *intersection = realloc(*intersection,k*sizeof(struct dirent*));
//    *i_len = k;
//    *dA_len = ii;
//    *dB_len = jj;
//}

void print_and_free(struct dirent ** arr, int len, char* name){
    printf("%s: ",name);
    for (int i = 0; i < len; i++) {
        printf("%s ",arr[i]->d_name);
        free(arr[i]);
    }
    free(arr);
    putchar('\n');
}

//void print_fts(FTSENT* list){
//    FTSENT* tmp = list;
//    char* type;
//
//    while(tmp){
//        switch (tmp->fts_info) {
//            case FTS_D:
//                type = "directory";
//                break;
//            case FTS_F:
//                type = "file";
//                break;
//            case FTS_SL:
//                type = "softlink";
//                break;
//            default:
//                type = "unknown";
//                break;
//        }
//        printf("name: %s, path: %s, access_path: %s, level: %d, type: %s\n",tmp->fts_name, tmp->fts_path,tmp->fts_accpath,tmp->fts_level,type);
//
//        tmp = tmp->fts_link;
//    }
//}
