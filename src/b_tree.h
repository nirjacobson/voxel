#ifndef B_TREE_H
#define B_TREE_H

#define B_TREE_PAGE_SIZE       1024
#define B_TREE_KEYS_PER_PAGE   ((B_TREE_PAGE_SIZE - sizeof(BTreeNodeHeader))/sizeof(BTreeEntry))

#include <unistd.h>

#include "chunk.h"

typedef struct {
  unsigned int freeSpacePtr;
  unsigned int rootPtr;
} BTreeHeader;

typedef struct {
  unsigned char isLeaf;
  unsigned int leftPtr;
  unsigned int numEntries;
} BTreeNodeHeader;

typedef struct {
  ChunkID key;
  unsigned int value;
  unsigned int rightPtr;
} BTreeEntry;

typedef struct {
  FILE* file;
} BTree;

BTree* b_tree_init(BTree* bt, const char* name);
void b_tree_destroy(BTree* btree);

void b_tree_init_tree(BTree* btree);

void b_tree_write_page(BTree* btree, unsigned int index, const char* page);
void b_tree_read_page(BTree* btree, unsigned int index, char* page);

BTreeHeader b_tree_get_header(BTree* btree);
void b_tree_set_header(BTree* btree, BTreeHeader* header);
BTreeNodeHeader b_tree_get_node_header(const char* page);
void b_tree_set_node_header(char* page, BTreeNodeHeader* header);

void b_tree_insert(BTree* btree, ChunkID* key, unsigned int value);
char b_tree_find(BTree* btree, ChunkID* key, unsigned int* valuePtr);
void b_tree_print(BTree* btree);


#endif // B_TREE_H
