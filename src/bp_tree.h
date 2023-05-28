#ifndef BP_TREE_H
#define BP_TREE_H

#define BP_TREE_PAGE_SIZE       1024
#define BP_TREE_KEYS_PER_PAGE   ((BP_TREE_PAGE_SIZE - sizeof(BPTreeNodeHeader))/sizeof(BPTreeEntry))

#include <unistd.h>

#include "chunk.h"

typedef struct {
  unsigned long freeSpacePtr;
  unsigned long rootPtr;
} BPTreeHeader;

typedef struct {
  unsigned char isLeaf;
  unsigned long leftPtr;
  unsigned int numEntries;
} BPTreeNodeHeader;

typedef struct {
  ChunkID key;
  unsigned long rightPtr;
} BPTreeEntry;

typedef struct {
  ChunkID key;
  unsigned long value;
} BPTreeLeafEntry;

typedef struct {
  FILE* file;
} BPTree;

BPTree* bp_tree_init(BPTree* bt, const char* name);
void bp_tree_destroy(BPTree* btree);

void bp_tree_init_tree(BPTree* btree);

void bp_tree_write_page(BPTree* btree, unsigned long address, const char* page);
void bp_tree_read_page(BPTree* btree, unsigned long address, char* page);

BPTreeHeader bp_tree_get_header(BPTree* btree);
void bp_tree_set_header(BPTree* btree, BPTreeHeader* header);
BPTreeNodeHeader bp_tree_get_node_header(const char* page);
void bp_tree_set_node_header(char* page, BPTreeNodeHeader* header);

void bp_tree_insert(BPTree* btree, ChunkID* key, unsigned long value);
char bp_tree_find(BPTree* btree, ChunkID* key, unsigned long* valuePtr);
void bp_tree_print(BPTree* btree);


#endif // BP_TREE_H