#define _GNU_SOURCE
#include "cmpcats.h"
#include "diff.h"
int main() {
    diff("dirA","dirB");
//    recursive_traverse("dirB","dirA");
    return 0;
}