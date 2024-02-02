//
// Created by vic on 27/01/2024.
//
#ifndef DELISOS_P4_LIST_H
#define DELISOS_P4_LIST_H
typedef struct list* listPtr;
//primary list functions
listPtr listInit(void);
void listPrint(listPtr);
//void listPrintall(listPtr );
void listInsert(listPtr , char *);
void listDstr(listPtr);
//int list_no_of_entries(listPtr );
#endif //DELISOS_P4_LIST_H
