//
// Created by vic on 25/01/2024.
//
#define _GNU_SOURCE
#include "cmpcats.h"

int main(int argc, char** argv){
//    DIR* dirA = opendir("dirA");
//    DIR* dirB = opendir("dirB");
    struct dirent** entriesA = NULL;
    struct dirent** entriesB = NULL;
    int lenA;
    int lenB;
    lenA = scandir("dirA",&entriesA,NULL,alphasort);
    lenB = scandir("dirB",&entriesB,NULL,alphasort);
    return 0;
}
