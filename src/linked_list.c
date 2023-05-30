#include "linked_list.h"
#include "internal/linked_list.h"

LinkedList* linked_list_init(LinkedList* l) {
  LinkedList* list = l ? l : NEW(LinkedList, 1);
  
  list->head = 0;
  list->tail = 0;
  list->size = 0;

  return list;
}

void linked_list_destroy(LinkedList* list, void (*destroy_fn)(void*)) {
  if (list->head == 0) {
    return;
  } else {
    LinkedListNode* node = list->head;
    while (node) {
      LinkedListNode* next = node->next;
      if (destroy_fn) {
        destroy_fn(node->data);
      }
      free(node);

      node = next;
    }
  }
}

void linked_list_node_init(LinkedListNode* node) {
  node->data = 0;
  node->prev = 0;
  node->next = 0;
}

void linked_list_insert(LinkedList* list, void* data) {
  LinkedListNode* node = NEW(LinkedListNode, 1);
  linked_list_node_init(node);

  node->data = data;

  if (list->tail == 0) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
  }

  list->size++;
}

void linked_list_insert_ordered(LinkedList* list, void* data, int (*comparison)(void*, void*)) {
  LinkedListNode* newNode = NEW(LinkedListNode, 1);
  linked_list_node_init(newNode);

  newNode->data = data;

  LinkedListNode* node;
  for (node = list->head; node; node = node->next) {
    if (comparison(data, node->data) < 0) {
      if (node == list->head) {
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
      } else {
        node->prev->next = newNode;
        newNode->prev = node->prev;
        newNode->next = node;
        node->prev = newNode;
      }

      list->size++;
      break;
    }
  }

  if (!node) {
    linked_list_insert(list, data);
  }
}


void linked_list_foreach(LinkedList* list, void (*visitor)(void*, void*), void* userData) {
  LinkedListNode* node = list->head;
  for (; node; node = node->next) {
    visitor(node->data, userData);
  }
}

LinkedListNode* linked_list_find(LinkedList* list, void* data, char (*equals_fn)(void*, void*)) {
  LinkedListNode* node = list->head;
  LinkedListNode* match = 0;
  for (; node; node = node->next) {
    if (equals_fn(data, node->data)) {
      match = node;
      break;
    }
  }

  return match;
}

void linked_list_remove(LinkedList* list, LinkedListNode* node, void (*destroy_fn)(void*)) {
  if (node) {
    if (node->prev) {
      node->prev->next = node->next;
    } else {
      list->head = node->next;
    }

    if (node->next) {
      node->next->prev = node->prev;
    } else {
      list->tail = node->prev;
    }

    list->size--;

    if (destroy_fn) {
      destroy_fn(node->data);
    }
    
    free(node);
  }
}

