#define _GNU_SOURCE
#include "cmpcats.h"
#include "diff.h"
int main(int argc, char** argv) {

    diff(argv[1],argv[2]);
//    recursive_traverse("dirB","dirA");
    return 0;
}