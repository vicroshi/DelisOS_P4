#ifndef DELISOS_P4_LIST_H
#define DELISOS_P4_LIST_H
typedef struct list* listPtr;
typedef struct list_node list_node;
//typedef struct list_node* list_node*;
struct list_node{
    char *file_path;
    ino_t st_inoA;
    ino_t st_inoB;
    mode_t st_mode;
    nlink_t st_nlink;
    off_t st_size;
    char is_merge;
//    struct stat* st; //xreiazetai gia to merge
    list_node *nxt;
};
typedef struct list list;
struct list{
    list_node* head,*tail; //kratame ton arxiko kai ton teliko kombo tis listas
    char * dirName;
//    int count;
    int nlinks_count;
};
char* listDirname(listPtr);
int listNlinksCount(listPtr);
//lis
//primary list functions
listPtr listInit(char*);
void listPrint(listPtr);
//void listPrintall(listPtr );
list_node* listInsert(listPtr , char *, struct stat*, char);
void listDstr(listPtr);
//int list_no_of_entries(listPtr );
#endif //DELISOS_P4_LIST_H
