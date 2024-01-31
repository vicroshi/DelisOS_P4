typedef struct list list;
typedef list* listPtr;
typedef struct list_node list_node;
typedef list_node* list_nodePtr;




//primary list functions
listPtr listInit(void);
void listPrint(listPtr);
void listPrintall(listPtr ); 
void listInsert(listPtr , char *);
void listDstr(listPtr);
int list_no_of_entries(listPtr );




