#ifndef DELISOS_P4_LIST_H
#define DELISOS_P4_LIST_H
typedef struct list* listPtr;
//typedef struct list_node list_node;
//typedef list_node* list_nodePtr;




//primary list functions
listPtr listInit(char *);
void listPrint(listPtr);
//void listPrintall(listPtr );
void listInsert(listPtr , char *);
void listDstr(listPtr);
//int list_no_of_entries(listPtr );
#endif //DELISOS_P4_LIST_H
