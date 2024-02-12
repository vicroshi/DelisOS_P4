#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "list.h"
//typedef struct list_node list_node;
////typedef struct list_node* list_node*;
//struct list_node{
//    char *file_path;
//    ino_t st_ino;
//    mode_t st_mode;
//    nlink_t st_nlink;
//    off_t st_size;
//    char is_merge;
////    struct stat* st; //xreiazetai gia to merge
//    list_node *nxt;
//};
//typedef struct list list;
//struct list{
//    list_node* head,*tail; //kratame ton arxiko kai ton teliko kombo tis listas
//    char * dirName;
////    int count;
//    int nlinks_count;
//};

char* listDirname(listPtr list){
    return list->dirName;
}

int listNlinksCount(listPtr list){
    return list->nlinks_count;
}

listPtr listInit(char* dirName){
    listPtr l;
    l=malloc(sizeof(list));
    if(l==NULL){
        perror("malloc()");
        exit(1);
    }
    l->head=NULL;
    l->tail=NULL;
    if(dirName!=NULL){
        l->dirName=malloc(strlen(dirName)+1);
        if(l->dirName==NULL){
            perror("malloc()");
            exit(1);
        }
        strcpy(l->dirName,dirName);
    }
    else{
        l->dirName = dirName;
    }
    l->nlinks_count = 0;
//    l->count=0;
    return l;
}

void listPrint(listPtr l){
    list_node* p=l->head;
    while(p!=NULL){
        printf("%s\n",p->file_path+strlen(l->dirName)+1); //+1 gia na min ektypwthei kai to '/'
        p=p->nxt;
    }
//    return;
}

list_node* listInsert(listPtr l, char *path, struct stat* st, char is_merge){
    list_node *new;
    new=malloc(sizeof(list_node));
    if(new==NULL){
        perror("malloc()");
        exit(1);
    }
    new->file_path=malloc((strlen(path)+1)*sizeof(char));
    if(new->file_path==NULL){
        perror("malloc()");
        exit(1);
    }
    strcpy(new->file_path,path);
    new->st_nlink = st->st_nlink;
    new->st_mode = st->st_mode;
    new->st_size = st->st_size;
    new->is_merge = is_merge;
    new->st_inoA = st->st_ino;
    new->st_inoB = 0;
    if(st->st_nlink>1){
        l->nlinks_count++;
    }
    new->nxt=NULL; //panta o neos kombos tha mpainei sto telos
    if(l->head==NULL){//i lista mas eiani keni, bazoume ton kombo pou ftiaksame stin arxi
        l->head=new;
        l->tail=new;
    }
    else{//den einai o prwtos kombos pou mpainei stin lista, tonbazoume sto telos
        l->tail->nxt=new; //ton kanoume to neo akro
        l->tail=new;
    }
    return new;
}

void listDstr(listPtr l){
    list_node* p,*tmp;
    p = l->head;
    free(l->dirName);
    while(p!=NULL){
        tmp = p;
        p = p->nxt;
        free(tmp->file_path);
        free(tmp);
    }
    free(l);
//    return;
}
