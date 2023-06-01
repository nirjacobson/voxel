#ifndef BP_TREE_INTERNAL_H
#define BP_TREE_INTERNAL_H

#define BP_TREE_PAGE_SIZE       1024
#define BP_TREE_KEYS_PER_PAGE   ((BP_TREE_PAGE_SIZE - sizeof(BPTreeNodeHeader))/sizeof(BPTreeEntry))

#include "../bp_tree.h"

typedef struct {
    unsigned long freeSpacePtr;
    unsigned long rootPtr;
} BPTreeHeader;

typedef struct {
    unsigned char isLeaf;
    unsigned long leftPtr;
    int numEntries;
} BPTreeNodeHeader;

typedef struct {
    ChunkID key;
    unsigned long rightPtr;
} BPTreeEntry;

typedef struct {
    ChunkID key;
    unsigned long value;
} BPTreeLeafEntry;

int compare_keys(ChunkID* keyA, ChunkID* keyB);

void bp_tree_init_tree(BPTree* bpTree);

void bp_tree_write_page(BPTree* bpTree, unsigned long address, const char* page);
unsigned long bp_tree_append_page(BPTree* bpTree, const char* page);
void bp_tree_read_page(BPTree* bpTree, unsigned long address, char* page);

BPTreeHeader bp_tree_get_header(BPTree* bpTree);
void bp_tree_set_header(BPTree* bpTree, BPTreeHeader* header);
BPTreeNodeHeader bp_tree_get_node_header(const char* page);
void bp_tree_set_node_header(char* page, BPTreeNodeHeader* header);

char* bp_tree_new_node(BPTreeEntry* entries, unsigned int count, unsigned long leftPtr);
char* bp_tree_new_leaf_node(BPTreeLeafEntry* entries, unsigned int count);

void bp_tree_insert_entry_sorted(BPTree* bpTree, unsigned long address, char* page, BPTreeEntry* entryToInsert);
void bp_tree_insert_leaf_entry_sorted(BPTree* bpTree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert);
char bp_tree_insert_leaf_entry_over(BPTree* bpTree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert);

BPTreeEntry* bp_tree_insert_split_insertion(BPTree* bpTree, unsigned long address, char* page, BPTreeEntry* entryToInsert);
BPTreeEntry* bp_tree_insert_split_leaf_insertion(BPTree* bpTree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert);

BPTreeEntry* bp_tree_insert_entry_helper(BPTree* bpTree, unsigned long address, BPTreeLeafEntry* entryToInsert);

char bp_tree_find_entry_in_leaf_page(const char* page, ChunkID* key, unsigned long* valuePtr);
char bp_tree_find_entry_helper(BPTree* bpTree, unsigned long address, ChunkID* key, unsigned long* valuePtr);

void bp_tree_print_helper(BPTree* bpTree, unsigned long address);

#endif // BP_TREE_INTERNAL_H