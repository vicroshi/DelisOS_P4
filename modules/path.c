//
// Created by vic on 2/12/24.
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "path.h"

//i sunartisi pairnei ena path kai ena file name kai epistrefei ena neo string me ta duo auta enwmena
char* construct_path(char* path,char* name){
    char *new_path=malloc(strlen(path)+strlen(name)+2); //+1 gia to '/' kai +1 gia to '/0'
    if(new_path==NULL){
        perror("malloc()");
        exit(1);
    }
    strcpy(new_path,path);
    strcat(new_path,"/");
    strcat(new_path,name);
    return new_path;
}
//edw pairnoume ena path me tyxon "." kai ".." mesa kai ta kanoume resolve
void canonicalize_path(char *path) {
    char result[PATH_MAX]; //edw tha kratisoume tis allages gia na tis parei pisw to path
    char *token, *saveptr;
    result[0] = '\0'; //kleinoume arxika to array gia na min exoume thema me uninitialized buffer
    token = strtok_r(path, "/", &saveptr); //pairnouome kathe substring anamesa se '/'
    while (token != NULL) {
        if (strcmp(token, "..") == 0) { //brethike "..", ara to proigoumeno substring apo auto prepei na bgei apo to path
            char *lastSlash = strrchr(result, '/'); //pame kai briskoume sto path pou exoume xtisei mexri twra poio einai to teleutiao substring psaxnontas gia to '/'.
            if (lastSlash != NULL) {//an to broume, kleinoume ekei to string gia na ginei overwrite meta apo kati allo
                *(lastSlash) = '\0';
            }
        } else if (strcmp(token, ".") != 0 && strlen(token) > 0) { //an brethei otidipote allo plin tou "." to eisagoume kanonika sto path
            strcat(result, "/");
            strcat(result, token);
        }
        token = strtok_r(NULL, "/", &saveptr);
    }
    if (path[0]=='/'){ //if the path was absolute, keep the starting '/'
        strcpy(path, result);
    }
    else{ //else skip it and don't copy it
        strcpy(path, result+1);
    }
}


char* relative_path(char* path,char* root){
    char* rel = strstr(path,root);
    if(rel!=NULL){
        return rel+strlen(root);
    }
    else{
        return rel;
    }
}

//edw antikathistoume allazoume apo to string haystack  tin prwti periptwsi pou briskoume to substring needle me to substring sub.
//xrisimiopoieitai gia tin merge gia na allazoume ta paths apo ta dirA kai dirB se dirC
char* substitute_path(char* haystack,char* needle, char* sub){
    char result[PATH_MAX];
    result[0] = '\0';
    char* str = strstr(haystack,needle);
    if (str == NULL){
        return str;
    }
    str+= strlen(needle);  //kratame to upoloipo substring meta apo to simeio pou tha ginei i allagi
    strcat(result,sub);
    strcat(result,str);
    char* ret = malloc(strlen(result)+1);
    strcpy(ret,result);
    return ret;
}
