#include "bp_tree.h"

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
  BPTree* btree = bt ? bt : NEW(BPTree, 1);

  char filename[16];
  sprintf(filename, "%s.vxl", name);
  if (access(filename, F_OK) == -1) {
    btree->file = fopen(filename, "w+b");
  } else {
    btree->file = fopen(filename, "r+b");
  }

  fseek(btree->file, 0, SEEK_END);
  if (ftell(btree->file) < sizeof(BPTreeHeader)) {
    bp_tree_init_tree(btree);
  }

  return btree;
}

void bp_tree_destroy(BPTree* btree) {
  fclose(btree->file);
}

void bp_tree_init_tree(BPTree* btree) {
  char page[BP_TREE_PAGE_SIZE];
  memset(page, 0, BP_TREE_PAGE_SIZE);

  BPTreeHeader btreeHeader = {
    .freeSpacePtr = sizeof(BPTreeHeader) + BP_TREE_PAGE_SIZE,
    .rootPtr = sizeof(BPTreeHeader)
  };

  memcpy(page, &btreeHeader, sizeof(BPTreeHeader));

  BPTreeNodeHeader rootHeader = { 1, 0, 0 };
  memcpy(page + sizeof(BPTreeHeader), &rootHeader, sizeof(BPTreeNodeHeader));

  bp_tree_write_page(btree, 0, page);
}

void bp_tree_write_page(BPTree* btree, unsigned long address, const char* page) {
  fseek(btree->file, address, SEEK_SET);
  fwrite(page, BP_TREE_PAGE_SIZE, 1, btree->file);
}

unsigned long bp_tree_append_page(BPTree* btree, const char* page) {
  BPTreeHeader btreeHeader = bp_tree_get_header(btree);
  unsigned long address = btreeHeader.freeSpacePtr;
  bp_tree_write_page(btree, address, page);

  btreeHeader.freeSpacePtr += BP_TREE_PAGE_SIZE;
  bp_tree_set_header(btree, &btreeHeader);

  return address;
}

void bp_tree_read_page(BPTree* btree, unsigned long address, char* page) {
  fseek(btree->file, address, SEEK_SET);
  fread(page, BP_TREE_PAGE_SIZE, 1, btree->file);
}

BPTreeHeader bp_tree_get_header(BPTree* btree) {
  char page[BP_TREE_PAGE_SIZE];
  bp_tree_read_page(btree, 0, page);

  BPTreeHeader header;
  memcpy(&header, page, sizeof(BPTreeHeader));

  return header;
}

void bp_tree_set_header(BPTree* btree, BPTreeHeader* header) {
  char page[BP_TREE_PAGE_SIZE];
  bp_tree_read_page(btree, 0, page);

  memcpy(page, header, sizeof(BPTreeHeader));

  bp_tree_write_page(btree, 0, page);
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

void bp_tree_insert_entry_sorted(BPTree* btree, unsigned long address, char* page, BPTreeEntry* entryToInsert) {
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
  bp_tree_write_page(btree, address, page);
}

void bp_tree_insert_leaf_entry_sorted(BPTree* btree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert) {
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
  bp_tree_write_page(btree, address, page);
}

char bp_tree_insert_leaf_entry_over(BPTree* btree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert) {
  BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);

  BPTreeLeafEntry entry;
  unsigned int offset = sizeof(BPTreeNodeHeader);
  for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BPTreeLeafEntry)) {
    memcpy(&entry, page+offset, sizeof(BPTreeLeafEntry));
    if (compare_keys(&entryToInsert->key, &entry.key) == 0) {
      ((BPTreeLeafEntry*)(page+offset))->value = entryToInsert->value;
      bp_tree_write_page(btree, address, page);

      return 1;
    }
  }

  return 0;
}

BPTreeEntry* bp_tree_insert_split_insertion(BPTree* btree, unsigned long address, char* page, BPTreeEntry* entryToInsert) {
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
  bp_tree_write_page(btree, address, page);

  unsigned long rightPageAddress = bp_tree_append_page(btree, rightPage);
  entryToInsertUp->rightPtr = rightPageAddress;

  free(rightPage);

  return entryToInsertUp;
}

BPTreeEntry* bp_tree_insert_split_leaf_insertion(BPTree* btree, unsigned long address, char* page, BPTreeLeafEntry* entryToInsert) {
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
  bp_tree_write_page(btree, address, page);

  unsigned int rightPageIndex = bp_tree_append_page(btree, rightPage);
  entryToInsertUp->rightPtr = rightPageIndex;

  free(rightPage);

  return entryToInsertUp;
}

BPTreeEntry* bp_tree_insert_entry_helper(BPTree* btree, unsigned long address, BPTreeLeafEntry* entryToInsert) {
  char page[BP_TREE_PAGE_SIZE];
  bp_tree_read_page(btree, address, page);

  BPTreeNodeHeader pageHeader = bp_tree_get_node_header(page);

  if (pageHeader.isLeaf) {
    if (!bp_tree_insert_leaf_entry_over(btree, address, page, entryToInsert)) {
      if (pageHeader.numEntries == BP_TREE_KEYS_PER_PAGE) {
        return bp_tree_insert_split_leaf_insertion(btree, address, page, entryToInsert);
      } else {
        bp_tree_insert_leaf_entry_sorted(btree, address, page, entryToInsert);
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
        BPTreeEntry* entryToInsertUp = bp_tree_insert_entry_helper(btree, i == 0 ? pageHeader.leftPtr : entry[1].rightPtr, entryToInsert);

        if (entryToInsertUp) {
          if (pageHeader.numEntries == BP_TREE_KEYS_PER_PAGE) {
            return bp_tree_insert_split_insertion(btree, address, page, entryToInsertUp);
          } else {
            bp_tree_insert_entry_sorted(btree, address, page, entryToInsertUp);
          }
        }

        return NULL;
      } else if (compare == 0) {
        break;
      }
    }

    BPTreeEntry* entryToInsertUp = bp_tree_insert_entry_helper(btree, entry[0].rightPtr, entryToInsert);
    
    if (entryToInsertUp) {
      if (pageHeader.numEntries == BP_TREE_KEYS_PER_PAGE) {
        return bp_tree_insert_split_insertion(btree, address, page, entryToInsertUp);
      } else {
        bp_tree_insert_entry_sorted(btree, address, page, entryToInsertUp);
      }
    }
  }

  return NULL;
}

void bp_tree_insert(BPTree* btree, ChunkID* key, unsigned long value) {
  BPTreeLeafEntry entryToInsert;
  entryToInsert.key = *key;
  entryToInsert.value = value;

  BPTreeHeader btreeHeader = bp_tree_get_header(btree);

  BPTreeEntry* entryToInsertUp = bp_tree_insert_entry_helper(btree, btreeHeader.rootPtr, &entryToInsert);

  if (entryToInsertUp) {
    char* newRoot = bp_tree_new_node(entryToInsertUp, 1, btreeHeader.rootPtr);

    unsigned long newRootPtr = bp_tree_append_page(btree, newRoot);
    btreeHeader = bp_tree_get_header(btree);
    btreeHeader.rootPtr = newRootPtr;
    bp_tree_set_header(btree, &btreeHeader);
  }
}

void bp_tree_print_helper(BPTree* btree, unsigned long address) {
  char page[BP_TREE_PAGE_SIZE];
  bp_tree_read_page(btree, address, page);

  BPTreeNodeHeader nodeHeader = bp_tree_get_node_header(page);
  
  printf("{ ");

  printf("\"left\": ");

  if (nodeHeader.leftPtr) {
    bp_tree_print_helper(btree, nodeHeader.leftPtr);
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
        bp_tree_print_helper(btree, entry.rightPtr);
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

void bp_tree_print(BPTree* btree) {
  BPTreeHeader btreeHeader = bp_tree_get_header(btree);
  bp_tree_print_helper(btree, btreeHeader.rootPtr);

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

char bp_tree_find_entry_helper(BPTree* btree, unsigned long address, ChunkID* key, unsigned long* valuePtr) {
  char page[BP_TREE_PAGE_SIZE];
  bp_tree_read_page(btree, address, page);

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
          return bp_tree_find_entry_helper(btree, i == 0 ? pageHeader.leftPtr : entry[1].rightPtr, key, valuePtr);
        } else if (comparison == 0) {
          break;
        }
      }
      return bp_tree_find_entry_helper(btree, entry[0].rightPtr, key, valuePtr);
  }
}

char bp_tree_find(BPTree* btree, ChunkID* key, unsigned long* valuePtr) {
  BPTreeHeader btreeHeader = bp_tree_get_header(btree);
  return bp_tree_find_entry_helper(btree, btreeHeader.rootPtr, key, valuePtr);
}
