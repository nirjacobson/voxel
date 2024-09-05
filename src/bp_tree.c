#include "bp_tree.h"
#include "internal/bp_tree.h"

int compare_keys(ChunkID* keyA, ChunkID* keyB) {
    if (keyA->x == keyB->x) {
        if (keyA->y == keyB->y) {
            return keyA->z - keyB->z;
        }
        return keyA->y - keyB->y;
    }
    return keyA->x - keyB->x;
}

BPTree* bp_tree_init(BPTree* bt, const char* name) {
    BPTree* bpTree = bt ? bt : NEW(BPTree, 1);

    char filename[128];
#ifdef _WIN32
    sprintf(filename, "%s%s%s%s.vxl", getenv("HOMEDRIVE"), getenv("HOMEPATH"), "/", name);
#else
    sprintf(filename, "%s%s%s.vxl", getenv("HOME"), "/", name);
#endif

#ifndef _WIN32
    if (access(filename, F_OK) == -1) {
#else
    if (_access(filename, F_OK) == -1) {
#endif
        bpTree->file = fopen(filename, "w+b");
    } else {
        bpTree->file = fopen(filename, "r+b");
    }

    fseek(bpTree->file, 0, SEEK_END);
    if (ftell(bpTree->file) < sizeof(BPTreeHeader)) {
        bp_tree_init_tree(bpTree);
    }

    return bpTree;
}

void bp_tree_destroy(BPTree* bpTree) {
    fclose(bpTree->file);
}

void bp_tree_init_tree(BPTree* bpTree) {
    BPTreeHeader bpTreeHeader = {
        .freeSpacePtr = sizeof(BPTreeHeader) + BP_TREE_PAGE_SIZE,
        .rootPtr = sizeof(BPTreeHeader)
    };

    char page[BP_TREE_PAGE_SIZE];
    memset(page, 0, BP_TREE_PAGE_SIZE);
    BPTreeNodeHeader nodeHeader = {
        .isLeaf = 1,
        .leftPtr = 0,
        .numEntries = 0
    };
    bp_tree_set_node_header(page, &nodeHeader);

    bp_tree_set_header(bpTree, &bpTreeHeader);
    bp_tree_write_page(bpTree, sizeof(BPTreeHeader), page);

}

void bp_tree_write_page(BPTree* bpTree, unsigned long address, const char* page) {
    fseek(bpTree->file, address, SEEK_SET);
    fwrite(page, BP_TREE_PAGE_SIZE, 1, bpTree->file);
    fflush(bpTree->file);
}

unsigned long bp_tree_append_page(BPTree* bpTree, const char* page) {
    BPTreeHeader bpTreeHeader = bp_tree_get_header(bpTree);
    unsigned long address = bpTreeHeader.freeSpacePtr;
    bp_tree_write_page(bpTree, address, page);

    bpTreeHeader.freeSpacePtr += BP_TREE_PAGE_SIZE;
    bp_tree_set_header(bpTree, &bpTreeHeader);

    return address;
}

void bp_tree_read_page(BPTree* bpTree, unsigned long address, char* page) {
    fseek(bpTree->file, address, SEEK_SET);
    fread(page, BP_TREE_PAGE_SIZE, 1, bpTree->file);
}

BPTreeHeader bp_tree_get_header(BPTree* bpTree) {
    BPTreeHeader header;
    fseek(bpTree->file, 0, SEEK_SET);
    fread(&header, sizeof(BPTreeHeader), 1, bpTree->file);

    return header;
}

void bp_tree_set_header(BPTree* bpTree, BPTreeHeader* header) {
    fseek(bpTree->file, 0, SEEK_SET);
    fwrite(header, sizeof(BPTreeHeader), 1, bpTree->file);
    fflush(bpTree->file);
}

BPTreeNodeHeader bp_tree_get_node_header(const char* page) {
    BPTreeNodeHeader header;
    memcpy(&header, page, sizeof(BPTreeNodeHeader));

    return header;
}

void bp_tree_set_node_header(char* page, BPTreeNodeHeader* header) {
    memcpy(page, header, sizeof(BPTreeNodeHeader));
}

char* bp_tree_new_node(BPTreeEntry* entries, unsigned int count, unsigned long leftPtr) {
    char* page = NEW(char, BP_TREE_PAGE_SIZE);
    memset(page, 0, BP_TREE_PAGE_SIZE);

    BPTreeNodeHeader header;
    header.isLeaf = 0;
    header.leftPtr = leftPtr;
    header.numEntries = count;

    bp_tree_set_node_header(page, &header);

    memcpy(page + sizeof(BPTreeNodeHeader), entries, count * sizeof(BPTreeEntry));

    return page;
}

char* bp_tree_new_leaf_node(BPTreeLeafEntry* entries, unsigned int count) {
    char* page = NEW(char, BP_TREE_PAGE_SIZE);
    memset(page, 0, BP_TREE_PAGE_SIZE);

    BPTreeNodeHeader header;
    header.isLeaf = 1;
    header.leftPtr = 0;
    header.numEntries = count;

    bp_tree_set_node_header(page, &header);

    memcpy(page + sizeof(BPTreeNodeHeader), entries, count * sizeof(BPTreeLeafEntry));

    return page;
}

void bp_tree_insert_entry_sorted(BPTree* bpTree, unsigned long address, char* page, BPTreeEntry* entryToInsert) {
    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);
    BPTreeEntry* allEntries = NEW(BPTreeEntry, pageHeader.numEntries + 1);
    memcpy(allEntries, page+sizeof(BPTreeNodeHeader), pageHeader.numEntries * sizeof(BPTreeEntry));
    allEntries[pageHeader.numEntries] = *entryToInsert;

    for (int i = pageHeader.numEntries - 1; i >= 0; i--) {
        if (compare_keys(&allEntries[i].key, &allEntries[i+1].key) > 0) {
            BPTreeEntry temp = allEntries[i+1];
            allEntries[i+1] = allEntries[i];
            allEntries[i] = temp;
        } else {
            break;
        }
    }

    pageHeader.numEntries++;
    memcpy(page + sizeof(BPTreeNodeHeader), allEntries, pageHeader.numEntries * sizeof(BPTreeEntry));

    bp_tree_set_node_header(page, &pageHeader);
    bp_tree_write_page(bpTree, address, page);
}

void bp_tree_insert_leaf_entry_sorted(BPTree* bpTree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert) {
    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);
    BPTreeLeafEntry* allEntries = NEW(BPTreeLeafEntry, pageHeader.numEntries + 1);
    memcpy(allEntries, page+sizeof(BPTreeNodeHeader), pageHeader.numEntries * sizeof(BPTreeLeafEntry));
    allEntries[pageHeader.numEntries] = *entryToInsert;

    for (int i = pageHeader.numEntries - 1; i >= 0; i--) {
        if (compare_keys(&allEntries[i].key, &allEntries[i+1].key) > 0) {
            BPTreeLeafEntry temp = allEntries[i+1];
            allEntries[i+1] = allEntries[i];
            allEntries[i] = temp;
        } else {
            break;
        }
    }

    pageHeader.numEntries++;
    memcpy(page + sizeof(BPTreeNodeHeader), allEntries, pageHeader.numEntries * sizeof(BPTreeLeafEntry));

    bp_tree_set_node_header(page, &pageHeader);
    bp_tree_write_page(bpTree, address, page);
}

char bp_tree_insert_leaf_entry_over(BPTree* bpTree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert) {
    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);

    BPTreeLeafEntry entry;
    unsigned int offset = sizeof(BPTreeNodeHeader);
    for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BPTreeLeafEntry)) {
        memcpy(&entry, page+offset, sizeof(BPTreeLeafEntry));
        if (compare_keys(&entryToInsert->key, &entry.key) == 0) {
            ((BPTreeLeafEntry*)(page+offset))->value = entryToInsert->value;
            bp_tree_write_page(bpTree, address, page);

            return 1;
        }
    }

    return 0;
}

BPTreeEntry* bp_tree_insert_split_insertion(BPTree* bpTree, unsigned long address, char* page, BPTreeEntry* entryToInsert) {
    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);
    BPTreeEntry* allEntries = NEW(BPTreeEntry, pageHeader.numEntries + 1);
    memcpy(allEntries, page+sizeof(BPTreeNodeHeader), pageHeader.numEntries * sizeof(BPTreeEntry));
    allEntries[pageHeader.numEntries] = *entryToInsert;

    for (int i = pageHeader.numEntries - 1; i >= 0; i--) {
        if (compare_keys(&allEntries[i].key, &allEntries[i+1].key) > 0) {
            BPTreeEntry temp = allEntries[i+1];
            allEntries[i+1] = allEntries[i];
            allEntries[i] = temp;
        } else {
            break;
        }
    }

    BPTreeEntry* entryToInsertUp = NEW(BPTreeEntry, 1);
    int middleIndex = (pageHeader.numEntries + 1)/2;
    *entryToInsertUp = allEntries[middleIndex];

    char* rightPage = bp_tree_new_node(&allEntries[middleIndex+1], pageHeader.numEntries - middleIndex, entryToInsertUp->rightPtr);

    pageHeader.numEntries = middleIndex;
    bp_tree_set_node_header(page, &pageHeader);
    bp_tree_write_page(bpTree, address, page);

    unsigned long rightPageAddress = bp_tree_append_page(bpTree, rightPage);
    entryToInsertUp->rightPtr = rightPageAddress;

    free(rightPage);

    return entryToInsertUp;
}

BPTreeEntry* bp_tree_insert_split_leaf_insertion(BPTree* bpTree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert) {
    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);
    BPTreeLeafEntry* allEntries = NEW(BPTreeLeafEntry, pageHeader.numEntries + 1);
    memcpy(allEntries, page+sizeof(BPTreeNodeHeader), pageHeader.numEntries * sizeof(BPTreeLeafEntry));
    allEntries[pageHeader.numEntries] = *entryToInsert;

    for (int i = pageHeader.numEntries - 1; i >= 0; i--) {
        if (compare_keys(&allEntries[i].key, &allEntries[i+1].key) > 0) {
            BPTreeLeafEntry temp = allEntries[i+1];
            allEntries[i+1] = allEntries[i];
            allEntries[i] = temp;
        } else {
            break;
        }
    }

    BPTreeEntry* entryToInsertUp = NEW(BPTreeEntry, 1);
    int middleIndex = (pageHeader.numEntries + 1)/2;
    entryToInsertUp->key = allEntries[middleIndex].key;

    char* rightPage = bp_tree_new_leaf_node(&allEntries[middleIndex], pageHeader.numEntries - middleIndex + 1);

    pageHeader.numEntries = middleIndex;
    bp_tree_set_node_header(page, &pageHeader);
    bp_tree_write_page(bpTree, address, page);

    unsigned int rightPageIndex = bp_tree_append_page(bpTree, rightPage);
    entryToInsertUp->rightPtr = rightPageIndex;

    free(rightPage);

    return entryToInsertUp;
}

BPTreeEntry* bp_tree_insert_entry_helper(BPTree* bpTree, unsigned long address, BPTreeLeafEntry* entryToInsert) {
    char page[BP_TREE_PAGE_SIZE];
    bp_tree_read_page(bpTree, address, page);

    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);

    if (pageHeader.isLeaf) {
        if (!bp_tree_insert_leaf_entry_over(bpTree, address, page, entryToInsert)) {
            if (pageHeader.numEntries == BP_TREE_KEYS_PER_PAGE) {
                return bp_tree_insert_split_leaf_insertion(bpTree, address, page, entryToInsert);
            } else {
                bp_tree_insert_leaf_entry_sorted(bpTree, address, page, entryToInsert);
            }
        }
    } else {
        BPTreeEntry entry[2] = {};
        unsigned int offset = sizeof(BPTreeNodeHeader);
        for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BPTreeEntry)) {
            entry[1] = entry[0];
            memcpy(&entry[0], page+offset, sizeof(BPTreeEntry));
            int compare = compare_keys(&entryToInsert->key, &entry[0].key);
            if (compare < 0) {
                BPTreeEntry* entryToInsertUp = bp_tree_insert_entry_helper(bpTree, i == 0 ? pageHeader.leftPtr : entry[1].rightPtr, entryToInsert);

                if (entryToInsertUp) {
                    if (pageHeader.numEntries == BP_TREE_KEYS_PER_PAGE) {
                        return bp_tree_insert_split_insertion(bpTree, address, page, entryToInsertUp);
                    } else {
                        bp_tree_insert_entry_sorted(bpTree, address, page, entryToInsertUp);
                    }
                }

                return NULL;
            } else if (compare == 0) {
                break;
            }
        }

        BPTreeEntry* entryToInsertUp = bp_tree_insert_entry_helper(bpTree, entry[0].rightPtr, entryToInsert);

        if (entryToInsertUp) {
            if (pageHeader.numEntries == BP_TREE_KEYS_PER_PAGE) {
                return bp_tree_insert_split_insertion(bpTree, address, page, entryToInsertUp);
            } else {
                bp_tree_insert_entry_sorted(bpTree, address, page, entryToInsertUp);
            }
        }
    }

    return NULL;
}

void bp_tree_insert(BPTree* bpTree, ChunkID* key, unsigned long value) {
    BPTreeLeafEntry entryToInsert;
    entryToInsert.key = *key;
    entryToInsert.value = value;

    BPTreeHeader bpTreeHeader = bp_tree_get_header(bpTree);

    BPTreeEntry* entryToInsertUp = bp_tree_insert_entry_helper(bpTree, bpTreeHeader.rootPtr, &entryToInsert);

    if (entryToInsertUp) {
        char* newRoot = bp_tree_new_node(entryToInsertUp, 1, bpTreeHeader.rootPtr);

        unsigned long newRootPtr = bp_tree_append_page(bpTree, newRoot);
        bpTreeHeader = bp_tree_get_header(bpTree);
        bpTreeHeader.rootPtr = newRootPtr;
        bp_tree_set_header(bpTree, &bpTreeHeader);
    }
}

char bp_tree_find_entry_in_leaf_page(const char* page, ChunkID* key, unsigned long* valuePtr) {
    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);

    BPTreeLeafEntry entry;
    BPTreeLeafEntry entryToFind;
    entryToFind.key = *key;
    unsigned int offset = sizeof(BPTreeNodeHeader);
    for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BPTreeLeafEntry)) {
        memcpy(&entry, page+offset, sizeof(BPTreeLeafEntry));
        if(compare_keys(&entryToFind.key, &entry.key) == 0) {
            *valuePtr = entry.value;
            return 1;
        }
    }

    return 0;
}

char bp_tree_find_entry_helper(BPTree* bpTree, unsigned long address, ChunkID* key, unsigned long* valuePtr) {
    char page[BP_TREE_PAGE_SIZE];
    bp_tree_read_page(bpTree, address, page);

    BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);

    if (pageHeader.isLeaf) {
        return bp_tree_find_entry_in_leaf_page(page, key, valuePtr);
    } else {
        BPTreeEntry entry[2];
        BPTreeEntry entryToFind;
        entryToFind.key = *key;
        unsigned int offset = sizeof(BPTreeNodeHeader);
        for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BPTreeEntry)) {
            entry[1] = entry[0];
            memcpy(&entry[0], page+offset, sizeof(BPTreeEntry));
            int comparison = compare_keys(&entryToFind.key, &entry[0].key);
            if (comparison < 0) {
                return bp_tree_find_entry_helper(bpTree, i == 0 ? pageHeader.leftPtr : entry[1].rightPtr, key, valuePtr);
            } else if (comparison == 0) {
                break;
            }
        }
        return bp_tree_find_entry_helper(bpTree, entry[0].rightPtr, key, valuePtr);
    }
}

char bp_tree_find(BPTree* bpTree, ChunkID* key, unsigned long* valuePtr) {
    BPTreeHeader bpTreeHeader = bp_tree_get_header(bpTree);
    return bp_tree_find_entry_helper(bpTree, bpTreeHeader.rootPtr, key, valuePtr);
}

void bp_tree_print_helper(BPTree* bpTree, unsigned long address) {
    char page[BP_TREE_PAGE_SIZE];
    bp_tree_read_page(bpTree, address, page);

    BPTreeNodeHeader nodeHeader = bp_tree_get_node_header(page);

    printf("{ ");

    printf("\"left\": ");

    if (nodeHeader.leftPtr) {
        bp_tree_print_helper(bpTree, nodeHeader.leftPtr);
    } else {
        printf("null");
    }

    printf(", ");

    unsigned int offset = sizeof(BPTreeNodeHeader);

    if (nodeHeader.isLeaf) {
        BPTreeLeafEntry entry;
        for (unsigned int i = 0; i < nodeHeader.numEntries; i++, offset += sizeof(BPTreeLeafEntry)) {
            memcpy(&entry, page+offset, sizeof(BPTreeLeafEntry));
            printf("\"%d, %d, %d\": %ld",
                   entry.key.x,
                   entry.key.y,
                   entry.key.z,
                   entry.value
                  );
            if (i < nodeHeader.numEntries - 1) {
                printf(", ");
            }
        }
    } else {
        BPTreeEntry entry;
        for (unsigned int i = 0; i < nodeHeader.numEntries; i++, offset += sizeof(BPTreeEntry)) {
            memcpy(&entry, page+offset, sizeof(BPTreeEntry));
            printf("\"%d, %d, %d\": ",
                   entry.key.x,
                   entry.key.y,
                   entry.key.z
                  );
            if (entry.rightPtr) {
                bp_tree_print_helper(bpTree, entry.rightPtr);
            } else {
                printf("null");
            }
            if (i < nodeHeader.numEntries - 1) {
                printf(", ");
            }
        }
    }

    printf(" }\n");
}

void bp_tree_print(BPTree* bpTree) {
    BPTreeHeader bpTreeHeader = bp_tree_get_header(bpTree);
    bp_tree_print_helper(bpTree, bpTreeHeader.rootPtr);

}
