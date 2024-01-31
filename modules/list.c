#include <stdio.h>
#include <stdlib.h>
#include "list.h" 
#include <string.h>
typedef struct list_node list_node;
typedef struct list_node* list_nodePtr;
struct list_node{
    char *file_path;
    list_node *nxt;
};
typedef struct list list;
struct list{
    list_nodePtr head;
};

listPtr listInit(void){
    listPtr l;
    l=malloc(sizeof(list));
    if(l==NULL){
        printf("Error in memory allocation, aborting.\n");
        exit(1);
    }
    l->head=NULL;
    return l;
}

void listPrint(listPtr l){
    list_nodePtr p=l->head;
    while(p!=NULL){
        printf("%s\n",p->file_path);
        p=p->nxt;
    }
//    return;
}

void listInsert(listPtr l, char *path){
    list_nodePtr p=l->head,new;
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
    new->nxt=p; //to bazoume stin arxi
    l->head=new;
//    return;
}

void listDstr(listPtr l){
    list_nodePtr p=l->head,prev;
   
    free(l);
    while(p!=NULL){
        prev=p;
        p=p->nxt;
        free(prev->file_path);
        free(prev);
    }
//    return;
}
