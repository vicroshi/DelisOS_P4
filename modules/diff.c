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
#define MERGE 1
#define NO_MERGE 0
int filter(const struct dirent* dir){
    return dir->d_name[0]!='.';
}

char* construct_path(char* path,char* name){
    char *new_path=malloc(strlen(path)+strlen(name)+2);
    if(new_path==NULL){
        perror("malloc()");
        exit(1);
    }
    strcpy(new_path,path);
    strcat(new_path,"/");
    strcat(new_path,name);
    return new_path;
}

char* construct_relative_path(char *path){//apo to full path, kratame mono to path mesa apo to folder
    char *relative_path;
    int index;
    for(index=0;index < strlen(path);index++) if(path[index++]=='/') break; //mexri to prwto '/' pou tha broume, ++ gia na pame ston xaraktira META to '/'
    relative_path=malloc((strlen(path)-index+1)*sizeof(char));
    if(relative_path==NULL){
        perror("malloc()");
        exit(1);
    }
    memcpy(relative_path,path+index,strlen(path)-index+1); //+1 gia to terminating character
    return relative_path;
}

//dinontas 2 paths arxeiwn, elegxw an exoun idio size kai idia contents
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

//#define dirname my_dirname

char* absolute_dirname(char* link,char* target){
    char* abs;

    char* lnk_base = strrchr(link,'/');
//    char* lnk_root = strchr(link,'/');
    int len_lnk = lnk_base!=NULL?strlen(lnk_base)-1:strlen(link);
    char* trg_base = strrchr(target,'/');
    int len_trg = trg_base!=NULL?strlen(trg_base):strlen(target);
    if(target[0] == '/'){
        char* targetdup = strdup(target);
        targetdup[strlen(target)-len_trg] = '\0';
//        printf("dup %s\n",targetdup);
        abs = realpath(targetdup,NULL);
//        printf("abs %s\n",abs);
        free(targetdup);
        return abs;
    }
    else {
        char pathc[PATH_MAX] = {};
//        printf("link: %s\ntarget: %s\n", link, target);
//        printf("%s\n", pathc);
        strncpy(pathc, link, strlen(link) - len_lnk);
//        printf("%s\n", pathc);
        strncat(pathc, target, strlen(target) - len_trg);
//        printf("%s\n", pathc);
//    char* dir = dirname(pathc);
//    printf("%s\n",dir);
        abs = realpath(pathc, NULL);
//        printf("%s\n",abs);
        if (abs == NULL) {
            perror("realpath");
            exit(EXIT_FAILURE);
        }
    }
//    }
    return abs;
}


void canonicalize_path(char *path) {
    char result[PATH_MAX];
    char *token, *saveptr;

    // Initialize result with an empty string
    result[0] = '\0';

    // Tokenize the input path based on '/'
    token = strtok_r(path, "/", &saveptr);

    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            // If token is '..', pop the last component from result
            char *lastSlash = strrchr(result, '/');
            if (lastSlash != NULL) {
                *(lastSlash) = '\0';
            }
        } else if (strcmp(token, ".") != 0 && strlen(token) > 0) {
            // Ignore '.' and add other components to result
            strcat(result, "/");
            strcat(result, token);
        }

        // Get the next token
        token = strtok_r(NULL, "/", &saveptr);
    }

    // Copy the canonicalized path back to the original buffer
    if (path[0]=='/'){
        strcpy(path, result);
    }
    else{
        strcpy(path, result+1);
    }

//    path[strlen(result)-1] = '\0';
}



int compare_links(char* pathA, char* rootA, char* pathB, char* rootB, struct stat* stA, struct stat* stB) {
//    char link_pathA[PATH_MAX];
//    char link_pathB[PATH_MAX];
    char targetA[PATH_MAX];
    char targetB[PATH_MAX];
//    strcpy(link_pathA, pathA);
//                        link_pathB = malloc(strlen(new_pathB)+1);
//    strcpy(link_pathB, pathB);
    char* link_pathA,* link_pathB;
    link_pathA = strdup(pathA);
    link_pathB = strdup(pathB);
    int flag = 1;
//    putchar('\n');
    while (flag) {
        char *target_pathA;
        char *target_pathB;
        int nbytesA, nbytesB;
        char *target_dirA;
        char *target_baseA;
        char *target_dirB;
        char *target_baseB;
        char *link_dirA;
        char *link_dirB;
        targetA[stA->st_size] = '\0';
//        printf("linkpaths:%s %s\n",link_pathA,link_pathB);
//        printf("paths:%s %s\n",pathA,pathB);
        nbytesA = readlink(link_pathA, targetA, stA->st_size + 1);
        targetA[nbytesA] = '\0';
//        printf("nbytesA = %d\n", nbytesA);
        if (nbytesA == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }

//        printf("targetA = %s\n",targetA);
//        if (targetA[0]!='/'){
//            target_dirA = absolute_dirname(link_pathA,targetA);
//        }
//        else{
//            target_dirA = dirname(targetA);
//        }
//        target_dirA = absolute_dirname(link_pathA, targetA);
        target_baseA = strrchr(targetA, '/');
        if (targetA[0] != '/'){
            target_pathA = construct_path(dirname(link_pathA),targetA);
        }
        else{
            target_pathA = strdup(targetA);
        }
        canonicalize_path(target_pathA);
        if(target_baseA == NULL){
            target_baseA = targetA;
        }
        else{
            target_baseA++;
        }
//        printf("dirname %s\n",target_dirA);
//        printf("dirname: %s basename: %s\n",target_dirA,targetA);
//        targetB[stB->st_size] = '\0';
        nbytesB = readlink(link_pathB, targetB, stB->st_size + 1);
        targetB[nbytesB] = '\0';
        if (nbytesB == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
//        printf("targetA = %s\n",targetA);
//        printf("targetB = %s\n",targetB);
//        printf("paths: %s %s\n",link_pathA,link_pathB);
//        printf("targets: %s %s\n",targetA,targetB);
        if (targetB[0] != '/'){
            target_pathB = construct_path(dirname(link_pathB),targetB);
        }
        else{
            target_pathB = strdup(targetB);
        }
        canonicalize_path(target_pathB);
        target_baseB = strrchr(targetB, '/'); // doesn't need free
        if (target_baseB == NULL){
            target_baseB = targetB;
        }
        else{
            target_baseB++;
        }
//        printf("dirname: %s basename: %s\n", strstr(target_dirB,rootB)+ strlen(rootB),target_baseB);
//        printf("dirname: %s basename: %s\n", strstr(target_dirA,rootA)+ strlen(rootA),target_baseA);
//        printf("dirname: %s basename: %s\n", target_dirA,targetA);
//        if (!strcmp(strstr(target_dirA,rootA)+ strlen(rootA), strstr(target_dirB,rootB)+ strlen(rootB)) && !strcmp(target_baseA, target_baseB)) {
//        printf("paths: %s %s\n",pathA,pathB);
//        printf("%s %s %d\n",target_pathA,target_pathB,strcmp(strstr(target_pathA,rootA)+ strlen(rootA),strstr(target_pathB,rootB)+strlen(rootB)));
        if (!strcmp(strstr(target_pathA,rootA)+ strlen(rootA),strstr(target_pathB,rootB)+strlen(rootB))) {
//            printf("here")
//            free(target_dirA);
//            free(target_dirB);
            struct stat st_targA, st_targB;
//            target_pathA = construct_path(target_dirA, target_baseA);
            lstat(target_pathA, &st_targA);
//            target_pathB = construct_path(target_dirB, target_baseB);
//            free(target_dirA);
//            free(target_dirB);
            lstat(target_pathB, &st_targB);
            if ((st_targA.st_mode & S_IFMT) == (st_targB.st_mode & S_IFMT))
                switch (st_targA.st_mode & S_IFMT) {
                    case S_IFREG:
//                        flag = 0;
                        flag = (st_targA.st_size == st_targB.st_size) &&
                               files_have_same_contents(target_pathA, target_pathB, st_targA.st_size);
                        free(link_pathA);
                        free(link_pathB);
                        free(target_pathA);
                        free(target_pathB);
                        return flag;
                    case S_IFLNK:
//                                        free(link_pathA);
//                                        link_pathA = malloc(strlen(target_pathA)+1);
                        free(link_pathA);
                        link_pathA = strdup(target_pathA);
//                                        free(link_pathB);
//                                        link_pathB = malloc(strlen(target_pathB)+1);
                        free(link_pathB);
                        link_pathB = strdup(target_pathB);
                        free(target_pathA);
                        free(target_pathB);
                        continue;
                    default:
                        return -1;
                        break;
                }
        }
        else {
//            printf("targetA_size: %d\ntargetB_size: %d\n", stA->st_size, stB->st_size);
//            printf("targetA: %s\ntargetB: %s\n", targetA, targetB);
//            free(target_dirA);
//            free(target_dirB);
            return 0;
        }
    }
}

#define compare_timespec(a, b, CMP)     (((a)->tv_sec==(b)->tv_sec) \
                                        ?((a)->tv_nsec CMP (b)->tv_nsec) \
                                        :((a)->tv_sec CMP (b)->tv_sec))


void compare(char* pathA, char* pathB,listPtr diffA,listPtr diffB, listPtr interscetion){
    int lenA = 0,lenB = 0;
    struct dirent** a = NULL;
    struct dirent** b = NULL;
    if (pathA){
        lenA = scandir(pathA,&a,filter,alphasort);
    }
    if (pathB){
        lenB = scandir(pathB,&b,filter,alphasort);
    }
    int i = 0, j = 0;
    char* new_pathA = NULL,*new_pathB = NULL;
    char* path;
    struct stat stA,stB;
    while(i<lenA && j<lenB){
        if(strcmp(a[i]->d_name,b[j]->d_name)<0){
            //lenA--;  //allagi sunthikis, to sbinw gia na kserw megethos pinaka gia free meta
            new_pathA = construct_path(pathA,a[i++]->d_name);
//            listInsert(interscetion,new_pathA);
            lstat(new_pathA,&stA);
            listInsert(diffA,new_pathA,&stA,MERGE);
            if( (stA.st_mode&S_IFMT) ==S_IFDIR){ //an einai dir, prepei na mpoume na to elegksoume
                compare(new_pathA, NULL,diffA,NULL,NULL);
            }
            free(new_pathA);
        }
        else if(strcmp(a[i]->d_name,b[j]->d_name)>0){
           // lenB--;
            new_pathB =  construct_path(pathB,b[j++]->d_name);
            lstat(new_pathB,&stB);
            listInsert(diffB, new_pathB,&stB,MERGE);
            if ((stB.st_mode&S_IFMT )== S_IFDIR){ //an einai dir, prepei na mpoume na to elegksoume
                compare(NULL,new_pathB,NULL,diffB,NULL);
            }
            free(new_pathB);
        }
        else{  //ta arxeia exoun to idio onoma, elegxw an exoun kai idio tupo. an diaferoun se kati, mpainoun kai stis duo listes
           // lenA--;
           // lenB--;
            new_pathA = construct_path(pathA,a[i++]->d_name);
            new_pathB = construct_path(pathB,b[j++]->d_name);
            lstat(new_pathA,&stA);
            lstat(new_pathB,&stB);
            if ((stA.st_mode&S_IFMT) == (stB.st_mode&S_IFMT)){
                int flag = 0;
                char link_pathA[PATH_MAX];
                char link_pathB[PATH_MAX];
//                int flag = 0;
                switch (stA.st_mode&S_IFMT) {
                    case S_IFDIR:
                        flag = 1;
                        compare(new_pathA,new_pathB,diffA,diffB,interscetion);
                        break;
                    case S_IFREG: //exoun idio name kai eiani regular files. elegxw an exoun idio size kai an ta contents einai ta idia
                        //compare files
                        flag = (stA.st_size==stB.st_size) && files_have_same_contents(new_pathA,new_pathB,stA.st_size);
                        if (!flag){
//                     = compare_timespec(&stA.st_mtim,&stB.st_mtim,>);
                            listInsert(diffA,new_pathA,&stA, compare_timespec(&stA.st_mtim,&stB.st_mtim,>));
                            listInsert(diffB,new_pathB,&stB,compare_timespec(&stA.st_mtim,&stB.st_mtim,<));
//                    "dirA/dir1/dir11/dir111/file.txt"
                        }
                        else{
                            listInsert(interscetion,new_pathA,&stA,MERGE);
                        }
//                        if( !(stA.st_size==stB.st_size && (files_have_same_contents(new_pathA,new_pathB,stA.st_size))) ){ //an den exoun idio size kai den exoun idia contents, den einai to idio arxeio, prepei na ta emfanisoume
////                            listInsert(diffA,new_pathA);
////                            listInsert(diffB,new_pathB);
//                            flag = 1;
//                        }
//                        else{ //einia idia vale to pathA apo paradoxi
//                            path = new_pathA;
//                        }
//                        listInsert(interscetion,path);
                        break;
                    case S_IFLNK:
                        //compare links
//                        link_pathA = malloc(strlen(new_pathA)+1);
//                        compare_links(new_pathA,new_pathB,&stA,&stB,diffA,)
                        flag = compare_links(new_pathA, listDirname(diffA),new_pathB,listDirname(diffB),&stA,&stB) ;
                        if (!flag){
//                     = compare_timespec(&stA.st_mtim,&stB.st_mtim,>);
                            listInsert(diffA,new_pathA,&stA, compare_timespec(&stA.st_mtim,&stB.st_mtim,>));
                            listInsert(diffB,new_pathB,&stB,compare_timespec(&stA.st_mtim,&stB.st_mtim,<));
//                    "dirA/dir1/dir11/dir111/file.txt"
                        }
                        else{
                            listInsert(interscetion,new_pathA,&stA,MERGE);
                        }
//                        if(!compare_links(new_pathA, listDirname(diffA),new_pathB,listDirname(diffB),&stA,&stB)){
//                            listInsert(diffA,new_pathA);
//                            listInsert(diffB,new_pathB);
//                        }
                        break;
                    default:
                        exit(EXIT_FAILURE);
                        break;
                }
            }
            free(new_pathA);
            free(new_pathB);
        }
    }
    for (; i<lenA; i++) {
        new_pathA = construct_path(pathA,a[i]->d_name);
        lstat(new_pathA,&stA);
        listInsert(diffA,new_pathA,&stA,MERGE);
        if ((stA.st_mode&S_IFMT )== S_IFDIR){
            compare(new_pathA,NULL,diffA,NULL,NULL);
        }
        free(new_pathA);
//        (*diffA)[ii] = malloc(a[i]->d_reclen);
//        memcpy((*diffA)[ii++],a[i],a[i]->d_reclen);
    }
    for (; j<lenB; j++) {
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


int search_inode(ino_t inode, ino_t* arr, int n){
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

//    char buffer;
//    size_t read_bytes,written_bytes;
//    read_bytes=read(fd_source,&buffer,1);
//    if(read_bytes==-1){
//        perror("read()");
//        exit(1);
//    }
//    while(read_bytes>0){
//        written_bytes=write(fd_dest,&buffer,1);
//        if(written_bytes==-1){
//            perror("write()");
//            exit(1);
//        }
//        read_bytes=read(fd_source,&buffer,1);
//        if(read_bytes==-1){
//            perror("read()");
//            exit(1);
//        }
//
//    }
    close(fd_source);
    close(fd_dest);
}


char* substitute_str(char* haystack,char* needle, char* sub){
    char result[PATH_MAX];
    result[0] = '\0';
    char* str = strstr(haystack,needle)+strlen(needle);
    strcat(result,sub);
    strcat(result,str);
    char* ret = malloc(strlen(result))+1;
    strcpy(ret,result);
    return ret;

}

int merge(char* dirC, listPtr* mergelists){
//    listPtr mergelists[] = {diffA,diffB,interscetion, NULL};
    ino_t* inodes; //inode array, keeps track of which inodes we have copied
    char** names; //names array
    int count; //array count
    int idx; //search index
    char* buffer;
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
//        count = 0;
//        inodes = malloc(sizeof(ino_t)*mergelists[i]->nlinks_count);
//        names = malloc(mergelists[i]->nlinks_count);
        while (tmp!=NULL){ //loop tis listas
            if(tmp->is_merge){
                merge_path = substitute_str(tmp->file_path,mergelists[i]->dirName,dirC);
                switch ((tmp->st_mode & S_IFMT)) {
                    case S_IFREG:
                        if (tmp->st_nlink>1){
                            if((idx = search_inode(tmp->st_ino,inodes,count))>=0){
                                link(names[idx],merge_path); //ftiaxnw to path me to dirC
                            }
                            else{
                                //create file
                                inodes[count] = tmp->st_ino;
                                names[count] = tmp->file_path; //ftiaxnw to path me to dirC
                                count++;
                            }
                        }
                        copy_file(tmp->file_path,merge_path,tmp->st_mode,tmp->st_size);
                        break;
                    case S_IFDIR:
                        //create dirs
                        break;
                    case S_IFLNK:
                        //create links
                        readlink(tmp->file_path,target,tmp->st_size);
                        symlink(merge_path,target); //alla ftiaxneis ta path me to dirC
                        break;
                }
                free(merge_path);
            }
        }
    }
    free(inodes);
    free(names);
}

listPtr* diff(char* dirA, char* dirB){
//    printf("%lu\n",ULONG_MAX);
//    printf("%d\n",sizeof(ino_t));

    listPtr diffA,diffB,interscetion;
    diffA = listInit(dirA);
    diffB = listInit(dirB);
    interscetion = listInit(NULL);
    compare(dirA,dirB,diffA,diffB,interscetion);
    printf("In %s:\n",dirA);
    listPrint(diffA);
    printf("\nIn %s:\n",dirB);
    listPrint(diffB);
//    listDstr(diffA);
//    listDstr(diffB);
    listPtr* ret = malloc(4*sizeof(list*));
    ret[0] = diffA;
    ret[1] = diffB;
    ret[2] = interscetion;
    ret[3] = NULL;
    return ret;
}
