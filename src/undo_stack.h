#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#include <stdbool.h>

#include "linked_list.h"

typedef struct Command Command;

struct Command {
    void* parent;
    void (*undo)(Command*);
    void (*redo)(Command*);
    int (*id)(void);
    bool (*merge)(Command*, Command*);
    void (*destroy)(Command*);
};

typedef struct {
    LinkedList commands;
    LinkedListNode* top;
} UndoStack;

void undo_stack_init(UndoStack* stack);
void undo_stack_destroy(UndoStack* stack);
void undo_stack_push(UndoStack* stack, Command* command);
void undo_stack_undo(UndoStack* stack);
void undo_stack_redo(UndoStack* stack);

#endif // UNDO_STACK_H