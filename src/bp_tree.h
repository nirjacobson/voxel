#ifndef BP_TREE_H
#define BP_TREE_H

#define BP_TREE_PAGE_SIZE       1024
#define BP_TREE_KEYS_PER_PAGE   ((BP_TREE_PAGE_SIZE - sizeof(BPTreeNodeHeader))/sizeof(BPTreeEntry))

#include <unistd.h>

#include "chunk.h"

typedef struct {
  FILE* file;
} BPTree;

BPTree* bp_tree_init(BPTree* bt, const char* name);
void bp_tree_destroy(BPTree* bpTree);

void bp_tree_insert(BPTree* bpTree, ChunkID* key, unsigned long value);
char bp_tree_find(BPTree* bpTree, ChunkID* key, unsigned long* valuePtr);
void bp_tree_print(BPTree* bpTree);

#endif // BP_TREE_H
