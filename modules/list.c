#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
typedef struct list_node list_node;
//typedef struct list_node* list_node*;
struct list_node{
    char *file_path;
    list_node *nxt;
};
typedef struct list list;
struct list{
    list_node* head,*tail; //kratame ton arxiko kai ton teliko kombo tis listas
    char * dirName;
};

listPtr listInit(char* dirName){
    listPtr l;
    l=malloc(sizeof(list));
    if(l==NULL){
        perror("malloc()");
        exit(1);
    }
    l->head=NULL;
    l->tail=NULL;
    l->dirName=malloc((strlen(dirName)+1)*sizeof(char));
    if(l->dirName==NULL){
        perror("malloc()");
        exit(1);
    }
    strcpy(l->dirName,dirName);
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

void listInsert(listPtr l, char *path){
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
    new->nxt=NULL; //panta o neos kombos tha mpainei sto telos
    if(l->head==NULL){//i lista mas eiani keni, bazoume ton kombo pou ftiaksame stin arxi
        l->head=new;
        l->tail=new;
    }
    else{//den einai o prwtos kombos pou mpainei stin lista, tonbazoume sto telos
        l->tail->nxt=new; //ton kanoume to neo akro
        l->tail=new;
    }



//    return;
}

void listDstr(listPtr l){
    list_node* p=l->head,*prev;
    free(l->dirName);
    free(l);
    while(p!=NULL){
        prev=p;
        p=p->nxt;
        free(prev->file_path);
        free(prev);
    }
//    return;
}
