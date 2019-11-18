#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "global.h"

typedef struct LinkedListNode {
  void* data;
  struct LinkedListNode* prev;
  struct LinkedListNode* next;
} LinkedListNode;

typedef struct {
  struct LinkedListNode* head;
  struct LinkedListNode* tail;
  int size;
} LinkedList;

LinkedList* linked_list_init(LinkedList* l);
void linked_list_destroy(LinkedList* list, void (*destroy_fn)(void*));

void linked_list_node_init(LinkedListNode* node);

void linked_list_insert(LinkedList* list, void* data);
void linked_list_insert_ordered(LinkedList* list, void* data, int (*comparison)(void*, void*));
void linked_list_foreach(LinkedList* list, void (*visitor)(void*, void*), void* userData);
LinkedListNode* linked_list_find(LinkedList* list, void* data, char (*equals_fn)(void*, void*));
void linked_list_remove(LinkedList* list, LinkedListNode* node, void (*destroy_fn)(void*));

#endif // LINKED_LIST_H
