#ifndef BP_TREE_H
#define BP_TREE_H

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "chunk.h"

#ifndef F_OK
#define F_OK    0
#endif

typedef struct {
    FILE* file;
} BPTree;

BPTree* bp_tree_init(BPTree* bt, const char* name);
void bp_tree_destroy(BPTree* bpTree);

void bp_tree_insert(BPTree* bpTree, ChunkID* key, unsigned long value);
char bp_tree_find(BPTree* bpTree, ChunkID* key, unsigned long* valuePtr);
void bp_tree_print(BPTree* bpTree);

#endif // BP_TREE_H
