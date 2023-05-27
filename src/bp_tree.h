#ifndef BP_TREE_H
#define BP_TREE_H

#define BP_TREE_PAGE_SIZE       1024
#define BP_TREE_KEYS_PER_PAGE   ((BP_TREE_PAGE_SIZE - sizeof(BPTreeNodeHeader))/sizeof(BPTreeEntry))

#include <unistd.h>

#include "chunk.h"

typedef struct {
  unsigned int freeSpacePtr;
  unsigned int rootPtr;
} BPTreeHeader;

typedef struct {
  unsigned char isLeaf;
  unsigned int leftPtr;
  unsigned int numEntries;
} BPTreeNodeHeader;

typedef struct {
  ChunkID key;
  unsigned int rightPtr;
} BPTreeEntry;

typedef struct {
  ChunkID key;
  unsigned int value;
} BPTreeLeafEntry;

typedef struct {
  FILE* file;
} BPTree;

BPTree* bp_tree_init(BPTree* bt, const char* name);
void bp_tree_destroy(BPTree* btree);

void bp_tree_init_tree(BPTree* btree);

void bp_tree_write_page(BPTree* btree, unsigned int index, const char* page);
void bp_tree_read_page(BPTree* btree, unsigned int index, char* page);

BPTreeHeader bp_tree_get_header(BPTree* btree);
void bp_tree_set_header(BPTree* btree, BPTreeHeader* header);
BPTreeNodeHeader bp_tree_get_node_header(const char* page);
void bp_tree_set_node_header(char* page, BPTreeNodeHeader* header);

void bp_tree_insert(BPTree* btree, ChunkID* key, unsigned int value);
char bp_tree_find(BPTree* btree, ChunkID* key, unsigned int* valuePtr);
void bp_tree_print(BPTree* btree);


#endif // BP_TREE_H
