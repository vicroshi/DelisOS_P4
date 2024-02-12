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
#include <sys/time.h>
#include <limits.h>
//#include <fts.h>
#include "diff.h"
#include "list.h"
#include "path.h"
#define MERGE 1
#define NO_MERGE 0
//sunartisi pou pairnei i scandir gia na min epistrepsei ta entries "." kai ".."
int filter(const struct dirent* dir){
    return dir->d_name[0]!='.';
}
//dinontas 2 paths arxeiwn, elegxw an exoun idia contents.
//i sunartisi tha klithei mono an exoun idio megethos, gi auto kai pernaw mono ena size san orisma
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


int compare_links(char* pathA, char* rootA, char* pathB, char* rootB, struct stat* stA, struct stat* stB) {
    char targetA[PATH_MAX];
    char targetB[PATH_MAX];
    char* link_pathA,* link_pathB; //this is the path for reading the link, at first its the same as pathA but changes to the target path when following a chain of links
    link_pathA = strdup(pathA);
    link_pathB = strdup(pathB);
    int flag = 1;
    while (flag) { //i started with a  `while(1)` loop so i left it like that :)
        //these will hold the readlink target path
        char *target_pathA;
        char *target_pathB;
        int nbytesA, nbytesB;
        targetA[stA->st_size] = '\0';
        nbytesA = readlink(link_pathA, targetA, stA->st_size + 1);
        targetA[nbytesA] = '\0';
        if (nbytesA == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }

        //NOTE: the link's path is relative to our current directory, the target's path is relative to the link's directory
        if (targetA[0] != '/'){ //if the link target is relative we need to construct it's path that is relative to our current directory
            target_pathA = construct_path(dirname(link_pathA),targetA); //this is going to be the directory path of the link concatinated with target
        }
        else{ //target is absolute no need to construct it
            target_pathA = strdup(targetA);
        }
        canonicalize_path(target_pathA); //this is to resolve extra slashes, dots, etc...
        //same as targetA
        nbytesB = readlink(link_pathB, targetB, stB->st_size + 1);
        targetB[nbytesB] = '\0';
        if (nbytesB == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        if (targetB[0] != '/'){
            target_pathB = construct_path(dirname(link_pathB),targetB);
        }
        else{
            target_pathB = strdup(targetB);
        }
        canonicalize_path(target_pathB);
        //we need to compare the relative paths, skip the roots
        if (!strcmp(strstr(target_pathA,rootA)+ strlen(rootA),strstr(target_pathB,rootB)+strlen(rootB))) {
            struct stat st_targA, st_targB;
            if(lstat(target_pathA, &st_targA)==-1){
                free(link_pathA);
                free(link_pathB);
                free(target_pathA);
                free(target_pathB);
                return 0;
            }
            if(lstat(target_pathB, &st_targB)==-1){
                free(link_pathA);
                free(link_pathB);
                free(target_pathA);
                free(target_pathB);
                return 0;
            }
            if ((st_targA.st_mode & S_IFMT) == (st_targB.st_mode & S_IFMT)) {
                switch (st_targA.st_mode & S_IFMT) {
                    case S_IFREG:
                        //no more links to follow just return the comparison check
                        flag = (st_targA.st_size == st_targB.st_size) &&
                               files_have_same_contents(target_pathA, target_pathB, st_targA.st_size);
                        free(link_pathA);
                        free(link_pathB);
                        free(target_pathA);
                        free(target_pathB);
                        return flag;
                    case S_IFLNK:
                        //the new link path will be the target  path, so we can follow links
                        free(link_pathA);
                        link_pathA = strdup(target_pathA);
                        free(link_pathB);
                        link_pathB = strdup(target_pathB);
                        free(target_pathA);
                        free(target_pathB);
                        continue;
                    default:
                        return 0;
                }
            }
            else{
                return 0;
            }
        }
        else {
            free(link_pathA);
            free(link_pathB);
            free(target_pathA);
            free(target_pathB);
            return 0;
        }
    }
}

//macro to find out which file was modified last
#define compare_timespec(a, b, CMP)     (((a)->tv_sec==(b)->tv_sec) \
                                        ?((a)->tv_nsec CMP (b)->tv_nsec) \
                                        :((a)->tv_sec CMP (b)->tv_sec))


void compare(char* pathA, char* pathB,listPtr diffA,listPtr diffB, listPtr interscetion){
    int lenA = 0,lenB = 0;
    struct dirent** a = NULL;
    struct dirent** b = NULL;
    //kaloume scandir gia na mas epistrafoun ola ta contents twn duo dirs taksinomimena kata onoma.
    //an dwthei san orisma ena apo ta duo NULL, den to kanoume giati auto simainei oti den uparxei to idio subdir sta duo dirs
    if (pathA){
        lenA = scandir(pathA,&a,filter,alphasort);
    }
    if (pathB){
        lenB = scandir(pathB,&b,filter,alphasort);
    }
    int i = 0, j = 0;
    char* new_pathA = NULL,*new_pathB = NULL;  //kathe fora pou sunantame ena neo arxeio, ftiaxnoume to path tou
    char* path;
    struct stat stA,stB;
    while(i<lenA && j<lenB){ //efoson oi duo pinakes einai sorted, pame kai elegxoume ta stoixeia ena pros ena
        if(strcmp(a[i]->d_name,b[j]->d_name)<0){ //o pinakas A exei stoixeio pou den exei o B, to krataw sti lista tou dirA
            new_pathA = construct_path(pathA,a[i++]->d_name); //auksanw mono to i pou einai o index tou pinaka a[]
            lstat(new_pathA,&stA);
            listInsert(diffA,new_pathA,&stA,MERGE);
            if( (stA.st_mode&S_IFMT) ==S_IFDIR){ //an einai dir, prepei na mpoume na to elegksoume
                compare(new_pathA, NULL,diffA,NULL,NULL); //anadromiko step
            }
            free(new_pathA);
        }
        else if(strcmp(a[i]->d_name,b[j]->d_name)>0){  //antistrofi periptwsi apo panw, antistoixi diadikasia.
           // lenB--;
            new_pathB =  construct_path(pathB,b[j++]->d_name); //auksanw mono to j pou einai o index tou pinaka b[]
            lstat(new_pathB,&stB);
            listInsert(diffB, new_pathB,&stB,MERGE);
            if ((stB.st_mode&S_IFMT )== S_IFDIR){
                compare(NULL,new_pathB,NULL,diffB,NULL);
            }
            free(new_pathB);
        }
        else{  //ta arxeia exoun to idio onoma, elegxw an exoun kai idio tupo. an diaferoun se kati, mpainoun kai stis duo listes alla mono to pio prosfato markarete me is_merge
            new_pathA = construct_path(pathA,a[i++]->d_name);
            new_pathB = construct_path(pathB,b[j++]->d_name);
            lstat(new_pathA,&stA);
            lstat(new_pathB,&stB);
            if ((stA.st_mode&S_IFMT) == (stB.st_mode&S_IFMT)){
                int flag = 0;
                switch (stA.st_mode&S_IFMT) {
                    case S_IFDIR:
                        flag = 1;
                        listInsert(interscetion,new_pathA, &stA,MERGE);
                        compare(new_pathA,new_pathB,diffA,diffB,interscetion);
                        break;
                    case S_IFREG: //exoun idio name kai eiani regular files. elegxw an exoun idio size kai an ta contents einai ta idia
                        //compare files
                        flag = (stA.st_size==stB.st_size) && files_have_same_contents(new_pathA,new_pathB,stA.st_size);
                        if (!flag){
                            listInsert(diffA,new_pathA,&stA, compare_timespec(&stA.st_mtim,&stB.st_mtim,>=));
                            listInsert(diffB,new_pathB,&stB,compare_timespec(&stA.st_mtim,&stB.st_mtim,<));
                        }
                        else{
                            if ((stA.st_nlink==1 && stB.st_nlink>1)||(stB.st_nlink==1 && stA.st_nlink>1)){
                                listInsert(diffA,new_pathA,&stA,stA.st_nlink==1);
                                listInsert(diffB,new_pathB,&stB,stB.st_nlink==1);
                            }
                            else{
                                list_node* node = listInsert(interscetion,new_pathA,&stA,MERGE);
                                if (stB.st_nlink>1){
                                    node->st_inoB = stB.st_ino;
                                }
                            }
                        }
                        break;
                    case S_IFLNK:
                        //compare links
                        flag = compare_links(new_pathA, listDirname(diffA),new_pathB,listDirname(diffB),&stA,&stB) ;
                        if (!flag){ //if they are not the same but have same name
                            listInsert(diffA,new_pathA,&stA, compare_timespec(&stA.st_mtim,&stB.st_mtim,>=)); //merge if latest
                            listInsert(diffB,new_pathB,&stB,compare_timespec(&stA.st_mtim,&stB.st_mtim,<)); //merge if latest
                        }
                        else{
                            listInsert(interscetion,new_pathA,&stA,MERGE); //they are the same put in intersection
                        }
                        break;
                    default:
                        exit(EXIT_FAILURE);
                }
            }
            free(new_pathA);
            free(new_pathB);
        }
    }
    for (; i<lenA; i++) { //put the rest in diffA
        new_pathA = construct_path(pathA,a[i]->d_name);
        lstat(new_pathA,&stA);
        listInsert(diffA,new_pathA,&stA,MERGE);
        if ((stA.st_mode&S_IFMT )== S_IFDIR){
            compare(new_pathA,NULL,diffA,NULL,NULL);
        }
        free(new_pathA);
    }
    for (; j<lenB; j++) { //put the rest in diffB
        new_pathB =  construct_path(pathB,b[j]->d_name);
        lstat(new_pathB,&stB);
        listInsert(diffB, new_pathB,&stB,MERGE);
        if ((stB.st_mode&S_IFMT )== S_IFDIR){
            compare(NULL,new_pathB,NULL,diffB,NULL);
        }
        free(new_pathB);
    }
    for(int i=0;i<lenA;i++) free(a[i]);
    free(a);
    for(int j=0;j<lenB;j++) free(b[j]);
    free(b);
}


int search_inode(ino_t inode, ino_t* arr, int n){ //linear array search
    for (int i = 0; i < n; i++) {
        if (arr[i]==inode){
            return i;
        }
    }
    return -1;
}

//dinoume to path sto opoio to arxeio tha antigraftei kai to onoma tou
void copy_file(char *source_file_path,char *dest_file_path,mode_t perms,size_t len){
    int fd_source,fd_dest;
    fd_source=open(source_file_path,O_RDONLY);
    if(fd_source==-1){
        perror("open()");
        exit(1);
    }
    fd_dest=open(dest_file_path, O_WRONLY | O_CREAT, (perms & 0777)); //ftiaxnoume to deytero arxeio me ta idia perms tou source file
    if(fd_dest==-1){
        printf("%s ",dest_file_path);
        perror("open()");
        exit(1);
    }
    ssize_t ret;
    do {
        ret = copy_file_range(fd_source,NULL,fd_dest,NULL,len,0);
        if (ret == -1){
            perror("copy_file_range");
            exit(EXIT_FAILURE);
        }
        len -= ret;
    } while (len>0&&ret>0);
    close(fd_source);
    close(fd_dest);
}




void merge(char* dirC, listPtr* mergelists){
    if (mkdir(dirC,0777)==-1){
        perror("mkdir");
        exit(EXIT_FAILURE);
    }
    ino_t* inodes; //inode array, keeps track of which inodes we have copied
    char** names; //names array
    int count=0; //array count
    int idx; //search index
    list_node* tmp;
    char target[PATH_MAX];
    int len = 0;
    for (int i = 0; i<3; i++) { //compiler will probably optimize it
        len+=mergelists[i]->nlinks_count;
    }
    inodes = malloc(sizeof(ino_t)*len);
    names = malloc(sizeof(char*)*len);
    char* merge_path;
    for (int i = 0; mergelists[i]!=NULL; i++) { //loop twn listwn
        tmp = mergelists[i]->head;
        while (tmp!=NULL){ //loop tis listas
            if(tmp->is_merge){
                merge_path = substitute_path(tmp->file_path,mergelists[i]->dirName,dirC); //create correct path for the merge directory
                if (merge_path==NULL){ //if null, skip it
                    tmp = tmp->nxt;
                    continue;
                }
                switch ((tmp->st_mode & S_IFMT)) {
                    case S_IFREG:
                        if (tmp->st_nlink>1){ //this is for hardlinks
                            if((idx = search_inode(tmp->st_inoA,inodes,count))>=0){ //if we have already copied linkode then link it
                                link(names[idx],merge_path); //ftiaxnw to path me to dirC
                            }
                            else{ //copy file and add its linknode
                                //create file
                                inodes[count] = tmp->st_inoA;
                                names[count] = strdup(merge_path);
                                count++;
                                //if there exists a second inode, its a special hardlinks case, just add this as well
                                if (tmp->st_inoB != 0 && search_inode(tmp->st_inoB,inodes,count)<0 ){
                                    inodes[count] = tmp->st_inoB;
                                    names[count] = strdup(merge_path);
                                    count++;
                                }
                                copy_file(tmp->file_path,merge_path,tmp->st_mode,tmp->st_size);
                            }
                        }
                        else{
                            copy_file(tmp->file_path,merge_path,tmp->st_mode,tmp->st_size);
                        }
                        break;
                    case S_IFDIR:
                        //create dirs
                        mkdir(merge_path,0777);
                        break;
                    case S_IFLNK:
                        //create links
                        readlink(tmp->file_path,target,tmp->st_size+1);
                        target[tmp->st_size] = '\0';
                        symlink(target,merge_path);
                        break;
                }
                free(merge_path);
            }
            tmp = tmp->nxt;
        }
    }
    free(inodes);
    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}

listPtr* diff(char* dirA, char* dirB){
    listPtr diffA,diffB,interscetion;
    diffA = listInit(dirA);
    diffB = listInit(dirB);
    interscetion = listInit(dirA);
    compare(dirA,dirB,diffA,diffB,interscetion);
    printf("In %s:\n",dirA);
    listPrint(diffA);
    printf("\nIn %s:\n",dirB);
    listPrint(diffB);
    listPtr* ret = malloc(4*sizeof(list*));
    ret[0] = interscetion;
    ret[1] = diffA;
    ret[2] = diffB;
    ret[3] = NULL; //this for mergelist loop
    return ret;
}
