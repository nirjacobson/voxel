#include "b_tree.h"

int compare_btree_entries(BTreeEntry* entryA, BTreeEntry* entryB) {
  if (entryA->key.x == entryB->key.x) {
    if (entryA->key.y == entryB->key.y) {
      return entryA->key.z - entryB->key.z;
    }
    return entryA->key.y - entryB->key.y;
  }
  return entryA->key.x - entryB->key.x;
}

BTree* b_tree_init(BTree* bt, const char* name) {
  BTree* btree = bt ? bt : NEW(BTree, 1);

  char filename[16];
  sprintf(filename, "%s.idx", name);
  if (access(filename, F_OK) == -1) {
    btree->file = fopen(filename, "w+b");
    b_tree_init_tree(btree);
  } else {
    btree->file = fopen(filename, "r+b");
  }

  return btree;
}

void b_tree_destroy(BTree* btree) {
  fclose(btree->file);
}

void b_tree_init_tree(BTree* btree) {
  char page[B_TREE_PAGE_SIZE];
  memset(page, 0, B_TREE_PAGE_SIZE);

  BTreeHeader btreeHeader = { 2, 1 };
  memcpy(page, &btreeHeader, sizeof(BTreeHeader));
  b_tree_write_page(btree, 0, page);
  
  memset(page, 0, B_TREE_PAGE_SIZE);

  BTreeNodeHeader rootHeader = { 1, 0, 0 };
  memcpy(page, &rootHeader, sizeof(BTreeNodeHeader));
  b_tree_write_page(btree, 1, page);
}

void b_tree_write_page(BTree* btree, unsigned int index, const char* page) {
  fseek(btree->file, index * B_TREE_PAGE_SIZE, SEEK_SET);
  fwrite(page, B_TREE_PAGE_SIZE, 1, btree->file);
}

unsigned int b_tree_append_page(BTree* btree, const char* page) {
  BTreeHeader btreeHeader = b_tree_get_header(btree);
  unsigned int index = btreeHeader.freeSpacePtr;
  b_tree_write_page(btree, index, page);

  btreeHeader.freeSpacePtr++;
  b_tree_set_header(btree, &btreeHeader);

  return index;
}

void b_tree_read_page(BTree* btree, unsigned int index, char* page) {
  fseek(btree->file, index * B_TREE_PAGE_SIZE, SEEK_SET);
  fread(page, B_TREE_PAGE_SIZE, 1, btree->file);
}


BTreeHeader b_tree_get_header(BTree* btree) {
  char page[B_TREE_PAGE_SIZE];
  b_tree_read_page(btree, 0, page);

  BTreeHeader header;
  memcpy(&header, page, sizeof(BTreeHeader));

  return header;
}

void b_tree_set_header(BTree* btree, BTreeHeader* header) {
  char page[B_TREE_PAGE_SIZE];
  b_tree_read_page(btree, 0, page);

  memcpy(page, header, sizeof(BTreeHeader));

  b_tree_write_page(btree, 0, page);
}

BTreeNodeHeader b_tree_get_node_header(const char* page) {
  BTreeNodeHeader header;
  memcpy(&header, page, sizeof(BTreeNodeHeader));

  return header;
}

void b_tree_set_node_header(char* page, BTreeNodeHeader* header) {
  memcpy(page, header, sizeof(BTreeNodeHeader));
}

char* b_tree_new_node(BTreeEntry* entries, unsigned int count, unsigned char isLeaf, unsigned int leftPtr) {
  char* page = NEW(char, B_TREE_PAGE_SIZE);
  memset(page, 0, B_TREE_PAGE_SIZE);

  BTreeNodeHeader header;
  header.isLeaf = isLeaf;
  header.leftPtr = leftPtr;
  header.numEntries = count;

  b_tree_set_node_header(page, &header);

  memcpy(page + sizeof(BTreeNodeHeader), entries, count * sizeof(BTreeEntry));

  return page;
}

void b_tree_insert_entry_sorted(BTree* btree, unsigned int pageIndex, char* page, BTreeEntry* entryToInsert) {
  BTreeNodeHeader pageHeader = b_tree_get_node_header(page);
  BTreeEntry* allEntries = NEW(BTreeEntry, pageHeader.numEntries + 1);
  memcpy(allEntries, page+sizeof(BTreeNodeHeader), pageHeader.numEntries * sizeof(BTreeEntry));
  allEntries[pageHeader.numEntries] = *entryToInsert;

  for (int i = pageHeader.numEntries - 1; i >= 0; i--) {
    if (compare_btree_entries(&allEntries[i], &allEntries[i+1]) > 0) {
      BTreeEntry temp = allEntries[i+1];
      allEntries[i+1] = allEntries[i];
      allEntries[i] = temp;
    } else {
      break;
    }
  }

  pageHeader.numEntries++;
  memcpy(page + sizeof(BTreeNodeHeader), allEntries, pageHeader.numEntries * sizeof(BTreeEntry));

  b_tree_set_node_header(page, &pageHeader);
  b_tree_write_page(btree, pageIndex, page);
}

char b_tree_insert_entry_over(BTree* btree, unsigned int pageIndex, char* page, BTreeEntry* entryToInsert) {
  BTreeNodeHeader pageHeader = b_tree_get_node_header(page);

  BTreeEntry entry;
  unsigned int offset = sizeof(BTreeNodeHeader);
  for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BTreeEntry)) {
    memcpy(&entry, page+offset, sizeof(BTreeEntry));
    if (compare_btree_entries(entryToInsert, &entry) == 0) {
      ((BTreeEntry*)(page+offset))->value = entryToInsert->value;
      b_tree_write_page(btree, pageIndex, page);

      return 1;
    }
  }

  return 0;
}


BTreeEntry* b_tree_insert_split_insertion(BTree* btree, unsigned int pageIndex, char* page, BTreeEntry* entryToInsert) {
  BTreeNodeHeader pageHeader = b_tree_get_node_header(page);
  BTreeEntry* allEntries = NEW(BTreeEntry, pageHeader.numEntries + 1);
  memcpy(allEntries, page+sizeof(BTreeNodeHeader), pageHeader.numEntries * sizeof(BTreeEntry));
  allEntries[pageHeader.numEntries] = *entryToInsert;

  for (int i = pageHeader.numEntries - 1; i >= 0; i--) {
    if (compare_btree_entries(&allEntries[i], &allEntries[i+1]) > 0) {
      BTreeEntry temp = allEntries[i+1];
      allEntries[i+1] = allEntries[i];
      allEntries[i] = temp;
    } else {
      break;
    }
  }

  BTreeEntry* entryToInsertUp = NEW(BTreeEntry, 1);
  int middleIndex = (pageHeader.numEntries + 1)/2;
  *entryToInsertUp = allEntries[middleIndex];
  
  char* leftPage = b_tree_new_node(&allEntries[0], middleIndex, pageHeader.isLeaf, pageHeader.leftPtr);
  char* rightPage = b_tree_new_node(&allEntries[middleIndex+1], pageHeader.numEntries - middleIndex, pageHeader.isLeaf, pageHeader.isLeaf ? 0 : entryToInsertUp->rightPtr);
  
  pageHeader.numEntries = middleIndex;
  b_tree_set_node_header(page, &pageHeader);
  b_tree_write_page(btree, pageIndex, page);

  unsigned int rightPageIndex = b_tree_append_page(btree, rightPage);
  entryToInsertUp->rightPtr = rightPageIndex;

  free(leftPage);
  free(rightPage);

  return entryToInsertUp;
}


BTreeEntry* b_tree_insert_entry_helper(BTree* btree, unsigned int pageIndex, BTreeEntry* entryToInsert) {
  entryToInsert->rightPtr = 0;
  char page[B_TREE_PAGE_SIZE];
  b_tree_read_page(btree, pageIndex, page);

  BTreeNodeHeader pageHeader = b_tree_get_node_header(page);

  if (pageHeader.isLeaf) {
    if (!b_tree_insert_entry_over(btree, pageIndex, page, entryToInsert)) {
      if (pageHeader.numEntries == B_TREE_KEYS_PER_PAGE) {
        return b_tree_insert_split_insertion(btree, pageIndex, page, entryToInsert);
      } else {
        b_tree_insert_entry_sorted(btree, pageIndex, page, entryToInsert);
      }
    }
  } else {
    if (!b_tree_insert_entry_over(btree, pageIndex, page, entryToInsert)) {
      if (pageHeader.numEntries == 0) {
        b_tree_insert_entry_sorted(btree, pageIndex, page, entryToInsert);
        return NULL;
      } else {
        BTreeEntry entry[2];
        unsigned int offset = sizeof(BTreeNodeHeader);
        for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BTreeEntry)) {
          entry[1] = entry[0];
          memcpy(&entry[0], page+offset, sizeof(BTreeEntry));
          if (compare_btree_entries(entryToInsert, &entry[0]) < 0) {
            BTreeEntry* entryToInsertUp = b_tree_insert_entry_helper(btree, i == 0 ? pageHeader.leftPtr : entry[1].rightPtr, entryToInsert);

            if (entryToInsertUp) {
              if (pageHeader.numEntries == B_TREE_KEYS_PER_PAGE) {
                return b_tree_insert_split_insertion(btree, pageIndex, page, entryToInsertUp);
              } else {
                b_tree_insert_entry_sorted(btree, pageIndex, page, entryToInsertUp);
              }
            }

            return NULL;
          }
        }

        BTreeEntry* entryToInsertUp = b_tree_insert_entry_helper(btree, entry[0].rightPtr, entryToInsert);
        
        if (entryToInsertUp) {
          if (pageHeader.numEntries == B_TREE_KEYS_PER_PAGE) {
            return b_tree_insert_split_insertion(btree, pageIndex, page, entryToInsertUp);
          } else {
            b_tree_insert_entry_sorted(btree, pageIndex, page, entryToInsertUp);
          }
        }

      }
    }
  }

  return NULL;

}

void b_tree_insert(BTree* btree, ChunkID* key, unsigned int value) {
  BTreeEntry entryToInsert;
  entryToInsert.key = *key;
  entryToInsert.value = value;
  entryToInsert.rightPtr = 0;

  BTreeHeader btreeHeader = b_tree_get_header(btree);

  BTreeEntry* entryToInsertUp = b_tree_insert_entry_helper(btree, btreeHeader.rootPtr, &entryToInsert);

  if (entryToInsertUp) {
    char* newRoot = b_tree_new_node(entryToInsertUp, 1, 0, btreeHeader.rootPtr);

    unsigned int newRootPtr = b_tree_append_page(btree, newRoot);
    btreeHeader = b_tree_get_header(btree);
    btreeHeader.rootPtr = newRootPtr;
    b_tree_set_header(btree, &btreeHeader);
  }
}

void b_tree_print_tree_helper(BTree* btree, unsigned int pageIndex) {
  char page[B_TREE_PAGE_SIZE];
  b_tree_read_page(btree, pageIndex, page);

  BTreeNodeHeader nodeHeader = b_tree_get_node_header(page);
  
  printf("{ ");

  printf("left: ");

  if (nodeHeader.leftPtr) {
    b_tree_print_tree_helper(btree, nodeHeader.leftPtr);
  } else {
    printf("null");
  }

  printf(", ");

  unsigned int offset = sizeof(BTreeNodeHeader);
  BTreeEntry entry;
  for (unsigned int i = 0; i < nodeHeader.numEntries; i++, offset += sizeof(BTreeEntry)) {
    memcpy(&entry, page+offset, sizeof(BTreeEntry));
    printf("\"%d, %d, %d: %d\": ",
      entry.key.x,
      entry.key.y,
      entry.key.z,
      entry.value
    );
    if (entry.rightPtr) {
      b_tree_print_tree_helper(btree, entry.rightPtr);
    } else {
      printf("null");
    }
    if (i < nodeHeader.numEntries - 1) {
      printf(", ");
    }
  }

  printf(" }\n");
}

void b_tree_print_tree(BTree* btree) {
  BTreeHeader btreeHeader = b_tree_get_header(btree);
  b_tree_print_tree_helper(btree, btreeHeader.rootPtr);

}

char b_tree_find_entry_in_page(const char* page, ChunkID* key, unsigned int* valuePtr) {
  BTreeNodeHeader pageHeader = b_tree_get_node_header(page);

  BTreeEntry entry;
  BTreeEntry entryToFind;
  entryToFind.key = *key;
  unsigned int offset = sizeof(BTreeNodeHeader);
  for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BTreeEntry)) {
    memcpy(&entry, page+offset, sizeof(BTreeEntry));
    if(compare_btree_entries(&entryToFind, &entry) == 0) {
      *valuePtr = entry.value;
      return 1;
    }
  }
  
  return 0;
}

char b_tree_find_entry_helper(BTree* btree, unsigned int pageIndex, ChunkID* key, unsigned int* valuePtr) {
  char page[B_TREE_PAGE_SIZE];
  b_tree_read_page(btree, pageIndex, page);

  BTreeNodeHeader pageHeader = b_tree_get_node_header(page);

  if (pageHeader.isLeaf) {
    return b_tree_find_entry_in_page(page, key, valuePtr);
  } else {
      BTreeEntry entry[2];
      BTreeEntry entryToFind;
      entryToFind.key = *key;
      unsigned int offset = sizeof(BTreeNodeHeader);
      for (unsigned int i = 0; i < pageHeader.numEntries; i++, offset += sizeof(BTreeEntry)) {
        entry[1] = entry[0];
        memcpy(&entry[0], page+offset, sizeof(BTreeEntry));
        int comparison = compare_btree_entries(&entryToFind, &entry[0]);
        if (comparison < 0) {
          return b_tree_find_entry_helper(btree, i == 0 ? pageHeader.leftPtr : entry[1].rightPtr, key, valuePtr);
        } else if (comparison == 0) {
          *valuePtr = entry[0].value;
          return 1;
        }
      }
      return b_tree_find_entry_helper(btree, entry[0].rightPtr, key, valuePtr);
  }
}

char b_tree_find(BTree* btree, ChunkID* key, unsigned int* valuePtr) {
  BTreeHeader btreeHeader = b_tree_get_header(btree);
  return b_tree_find_entry_helper(btree, btreeHeader.rootPtr, key, valuePtr);
}
