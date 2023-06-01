#ifndef APPLICATION_H
#define APPLICATION_H

#include "global.h"

struct Application;
struct Window;

typedef void (*ApplicationFn)(struct Application*);

typedef struct Application {
    struct Window* window;

    ApplicationFn setup;
    ApplicationFn main;
    ApplicationFn resize;
    ApplicationFn teardown;

    void* owner;
} Application;

Application* application_init(Application* a, void* owner, ApplicationFn setup, ApplicationFn main, ApplicationFn resize, ApplicationFn teardown);


#endif // APPLICATION_H
