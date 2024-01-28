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
int filter(const struct dirent* dir){
    return dir->d_name[0]!='.';
}

void cmp_ent(struct dirent** a, struct dirent**b, int a_len, int b_len, struct dirent*** intersection,struct dirent*** diffA,struct dirent*** diffB,int* i_len, int* dA_len, int* dB_len){
    int max = (a_len > b_len ? a_len : b_len);
//    printf("max %d\n",max);
//    for (int i = 0; i < a_len; i++) {
//        printf("size:%ld %s %d %ld %ld %d\n", sizeof(struct dirent),a[i]->d_name,a[i]->d_type,a[i]->d_ino,a[i]->d_off,a[i]->d_reclen);
//        free(a[i]);
//    }
    *intersection = malloc(sizeof(struct dirent*) * max);
    *diffA = malloc(sizeof(struct dirent*) * a_len);
    *diffB = malloc(sizeof(struct dirent*) * b_len);
//    for (int i = 0; i < max; i++) {
//        printf("i:%d ",i);
//        (*intersection)[i] = malloc(sizeof(struct dirent));
//        if (i<a_len){
//            printf("i:%d ",i);
//            (*diffA)[i] = malloc(sizeof(struct dirent));
//        }
//        if(i<b_len){
//
//            (*diffB)[i] = malloc(sizeof(struct dirent));
//        }
//    }
    putchar('\n');
    int i = 0, j = 0, k = 0, ii = 0, jj = 0;
    while (i < a_len && j < b_len){
        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
        if(strcmp(a[i]->d_name,b[j]->d_name)<0){
//            printf("ii:%d ",ii);
//            struct dirent da;
//            memcpy((*(*diffA)[ii]).d_name,(*a[i]).d_name,strlen((*a[i]).d_name)+1);
//            (*(*diffA)[ii]).d_reclen = (*a[i]).d_reclen;
//            (*(*diffA)[ii]).d_ino = (*a[i]).d_ino;
//            (*(*diffA)[ii]).d_off = (*a[i]).d_off;
//            (*(*diffA)[ii]).d_type = (*a[i]).d_type;
            (*diffA)[ii] = malloc(a[i]->d_reclen);
            memcpy((*diffA)[ii],a[i],a[i]->d_reclen);
            ii++;
            i++;
        }
        else if(strcmp(a[i]->d_name,b[j]->d_name)>0){
            (*diffB)[jj] = malloc(b[i]->d_reclen);
            *(*diffB)[jj++] = *b[j];
            j++;
        }
        else{
//            *(*intersection)[k++] = *a[i];
            (*intersection)[k] = malloc(a[i]->d_reclen);
            memcpy((*intersection)[k],a[i],a[i]->d_reclen);
//            memcpy((*(*intersection)[k]).d_name,(*a[i]).d_name,strlen((*a[i]).d_name)+1);
//            (*(*intersection)[k]).d_reclen = (*a[i]).d_reclen;
//            (*(*intersection)[k]).d_ino = (*a[i]).d_ino;
//            (*(*intersection)[k]).d_off = (*a[i]).d_off;
//            (*(*intersection)[k]).d_type = (*a[i]).d_type;
//            memcpy((*intersection)[k],a[i], sizeof(struct dirent),sizeof(a[i], sizstrlen(a[i], sizeof(struct diren+1t))
            i++;
            j++;
        }
    }
    for (; i < a_len; i++) {
        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
        memcpy((*diffA)[ii],a[i],a[i]->d_reclen);
//        *(*diffA)[ii++] = *a[i];
//        memcpy((*(*diffA)[ii]).d_name,(*a[i]).d_name,strlen((*a[i]).d_name)+1);
//        (*(*diffA)[ii]).d_reclen = (*a[i]).d_reclen;
//        (*(*diffA)[ii]).d_ino = (*a[i]).d_ino;
//        (*(*diffA)[ii]).d_off = (*a[i]).d_off;
//        (*(*diffA)[ii]).d_type = (*a[i]).d_type;
        ii++;
    }
    for (; j < b_len; j++) {
        printf("i=%d j=%d k=%d ii=%d jj=%d\n",i,j,k,ii,jj);
        *(*diffB)[jj++] = *b[j];
    }
//    *diffA = realloc(*diffA,ii*sizeof(struct dirent*));
//    *diffB = realloc(*diffB,jj*sizeof(struct dirent*));
//    *intersection = realloc(*intersection,k*sizeof(struct dirent*));
    *i_len = k;
    *dA_len = ii;
    *dB_len = jj;
}

char diff(char* dirA, char* dirB){
    struct dirent** entriesA;
    struct dirent** entriesB;
    int lenB = scandir(dirB,&entriesB,filter,alphasort);
    int lenA = scandir(dirA,&entriesA,filter,alphasort);
    printf("lenA=%d lenB=%d",lenA,lenB);

    struct dirent** intersection;
    struct dirent** diffA;
    struct dirent** diffB;
    int lenIntr;
    int lenDiffA;
    int lenDiffB;
    cmp_ent(entriesA,entriesB,lenA,lenB,
            &intersection,&diffA,&diffB,&lenIntr,&lenDiffA,&lenDiffB);
    printf("intersection: ");
    for (int i = 0; i < lenIntr; i++) {
        printf("%s ",intersection[i]->d_name);
    }
    putchar('\n');
    printf("diffA: ");
    for (int i = 0; i < lenDiffA; i++) {
        printf("%s ",diffA[i]->d_name);
    }
    putchar('\n');
    printf("diffB: ");
    for (int i = 0; i < lenDiffB; i++) {
        printf("%s ",diffB[i]->d_name);
    }
    putchar('\n');
    for (int i = 0; i < lenB; i++) {
        free(entriesB[i]);
    }
    free(entriesB);
    for (int i = 0; i < lenA; i++) {
//        printf("%s %d %ld %ld %d\n",entriesA[i]->d_name,entriesA[i]->d_type,entriesA[i]->d_ino,entriesA[i]->d_off,entriesA[i]->d_reclen);
        free(entriesA[i]);
    }
    free(entriesA);
//    for (int i = 0; i < lenDiffA; i++) {
    for (int i = 0; i < lenA; i++) {
        free(diffA[i]);
    }
    free(diffA);
//    for (int i = 0; i < lenDiffB; i++) {
    for (int i = 0; i < lenB; i++) {
        free(diffB[i]);
    }
    free(diffB);
    for (int i = 0; i < 6; i++) {
        free(intersection[i]);
    }
    free(intersection);
    return 1;
}


