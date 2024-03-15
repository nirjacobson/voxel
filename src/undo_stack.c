#include "undo_stack.h"

void destroy_command(void* commandPtr) {
    Command* command = (Command*)commandPtr;

    command->destroy(command);

    free(command->parent);
}

void undo_stack_init(UndoStack* stack) {
    linked_list_init(&stack->commands);
}

void undo_stack_destroy(UndoStack* stack) {
    linked_list_destroy(&stack->commands, destroy_command);
}

void undo_stack_push(UndoStack* stack, Command* command) {
    command->redo(command);

    if (!(stack->commands.size == 0 || 
          stack->top == 0)) {
        Command* topCommand = (Command*)stack->top->data;

        if (topCommand->id() != 0 && topCommand->id() == command->id()) {
            topCommand->merge(topCommand, command);
            return;
        }
    }

    linked_list_insert(&stack->commands, command);
    stack->top = stack->commands.tail;
}

void undo_stack_undo(UndoStack* stack) {
    if (stack->commands.size == 0 || stack->top == 0) {
        return;
    }

    Command* topCommand = (Command*)stack->top->data;

    topCommand->undo(topCommand);

    stack->top = stack->top->prev;
}

void undo_stack_redo(UndoStack* stack) {
    if (stack->commands.size == 0) {
        return;
    }
    
    Command* command;
    if (stack->top == 0) {
        command = (Command*)stack->commands.head->data;
    } else {
        if (stack->top->next) {
            command = (Command*)stack->top->next->data;
        } else {
            return;
        }
    }

    command->redo(command);

    if (stack->top == 0) {
        stack->top = stack->commands.head;
    } else {
        stack->top = stack->top->next;
    }
}