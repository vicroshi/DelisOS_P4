//
// Created by vic on 27/01/2024.
//


#define _GNU_SOURCE
#include "diff.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fts.h>
int filter(const struct dirent* dir){
    return dir->d_name[0]!='.';
}

int cmp_dir(char* pathA, char* pathB /**/){
    struct dirent** entriesA;
    struct dirent** entriesB;
    int lenA = scandir(pathA,entriesA,filter,alphasort);
    int lenB = scandir(pathB,entriesB,filter,alphasort);

}

void cmp_ent(char* pathA, char* pathB, struct dirent** a, struct dirent**b, int a_len, int b_len, struct dirent*** intersection,struct dirent*** diffA,struct dirent*** diffB,int* i_len, int* dA_len, int* dB_len){
    int max = (a_len > b_len ? a_len : b_len);
    *intersection = malloc(sizeof(struct dirent*) * max);
    *diffA = malloc(sizeof(struct dirent*) * a_len);
    *diffB = malloc(sizeof(struct dirent*) * b_len);
    int i = 0, j = 0, k = 0, ii = 0, jj = 0;
    struct stat stA,stB;
    char* cwd;
    while (i < a_len && j < b_len){
//        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
        if(strcmp(a[i]->d_name,b[j]->d_name)<0){
            (*diffA)[ii] = malloc(a[i]->d_reclen);
            memcpy((*diffA)[ii++],a[i],a[i]->d_reclen);
            i++;
        }
        else if(strcmp(a[i]->d_name,b[j]->d_name)>0){
            (*diffB)[jj] = malloc(b[i]->d_reclen);
            memcpy((*diffB)[jj++],b[j],b[j]->d_reclen);
            j++;
        }
        else{
            /*
             * stat gia elegxo tipou kai inode klp
             * switch gia to type
             * kai meta compares
             */
            cwd = get_current_dir_name();
            chdir(pathA);
            lstat(a[i]->d_name,&stA);
            printf("%s %ld\n",a[i]->d_name,stA.st_ino);
            chdir(cwd);
            chdir(pathB);
            lstat(b[j]->d_name,&stB);
            printf("%s %ld\n",b[j]->d_name,stB.st_ino);
            mode_t typeA = stA.st_mode & S_IFMT;
            mode_t typeB = stB.st_mode & S_IFMT;
            if (typeA == typeB){
                switch (typeA) {
                    case S_IFDIR:
                        //cmpdir
                        break;
                    case S_IFREG:
                        //cmpfile
                        break;
                    case S_IFLNK:
                        //cmplink
                        break;
                    default:
                        break;
                }
            }
            else{
                //einai diaforetika valta sto diff
            }

//            lstat()
//            lstat()
            (*intersection)[k] = malloc(a[i]->d_reclen);
            memcpy((*intersection)[k++],a[i],a[i]->d_reclen);
            i++;
            j++;
        }
    }
    for (; i < a_len; i++) {
//        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
        (*diffA)[ii] = malloc(a[i]->d_reclen);
        memcpy((*diffA)[ii++],a[i],a[i]->d_reclen);
    }
    for (; j < b_len; j++) {
//        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
        (*diffB)[jj] = malloc(b[j]->d_reclen);
        memcpy((*diffB)[jj++],b[j],b[j]->d_reclen);
    }
    //isws kai na mh xreiazetai kan den glittwneis xwros
//    *diffA = realloc(*diffA,ii*sizeof(struct dirent*));
//    *diffB = realloc(*diffB,jj*sizeof(struct dirent*));
//    *intersection = realloc(*intersection,k*sizeof(struct dirent*));
    *i_len = k;
    *dA_len = ii;
    *dB_len = jj;
}

void print_and_free(struct dirent ** arr, int len, char* name){
    printf("%s: ",name);
    for (int i = 0; i < len; i++) {
        printf("%s ",arr[i]->d_name);
        free(arr[i]);
    }
    free(arr);
    putchar('\n');

}

void print_fts(FTSENT* list){
    FTSENT* tmp = list;
    char* type;
    
    while(tmp){
        switch (tmp->fts_info) {
            case FTS_D:
                type = "directory";
                break;
            case FTS_F:
                type = "file";
                break;
            case FTS_SL:
                type = "softlink";
                break;
            default:
                type = "unknown";
                break;
        }
        printf("name: %s, path: %s, access_path: %s, level: %d, type: %s\n",tmp->fts_name, tmp->fts_path,tmp->fts_accpath,tmp->fts_level,type);

        tmp = tmp->fts_link;
    }
}

char diff(char* dirA, char* dirB){
    //dirent api
    struct dirent** entriesA;
    struct dirent** entriesB;
    int lenB = scandir(dirB,&entriesB,filter,alphasort);
    int lenA = scandir(dirA,&entriesA,filter,alphasort);
    struct dirent** intersection;
    struct dirent** diffA;
    struct dirent** diffB;
    int lenIntr;
    int lenDiffA;
    int lenDiffB;
    cmp_ent(dirA,dirB,entriesA,entriesB,lenA,lenB,
            &intersection,&diffA,&diffB,&lenIntr,&lenDiffA,&lenDiffB);
    print_and_free(entriesA,lenA,"dirA");
    print_and_free(entriesB,lenB,"dirB");
    print_and_free(diffA,lenDiffA,"diffA");
    print_and_free(diffB,lenDiffB,"diffB");
    print_and_free(intersection,lenIntr,"intersection");
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


