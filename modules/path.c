//
// Created by vic on 2/12/24.
//
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include "path.h"

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

char* substitute_path(char* haystack,char* needle, char* sub){
    char result[PATH_MAX];
    result[0] = '\0';
    char* str = strstr(haystack,needle);
    if (str == NULL){
        return NULL;
    }
    str+= strlen(needle);
    strcat(result,sub);
    strcat(result,str);
    char* ret = malloc(strlen(result)+1);
    strcpy(ret,result);
//    printf("strsub: %s\n",ret);
//    printf("haystack: %s\n",haystack);
//    printf("needle: %s\n",needle);
//    printf("result: %s\n",result);
//    printf("str: %s\n",str);
    return ret;

}
