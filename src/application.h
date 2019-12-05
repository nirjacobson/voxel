#ifndef APPLICATION_H
#define APPLICATION_H

#include "global.h"

struct Application;
struct Window;
struct Voxel;

typedef void (*ApplicationFn)(struct Application*);

struct Application {
  struct Window* window;
  
  ApplicationFn setup;
  ApplicationFn main;
  ApplicationFn resize;
  ApplicationFn teardown;

  struct Voxel* voxel;
};

typedef struct Application Application;

Application* application_init(Application* a, struct Voxel* voxel, ApplicationFn setup, ApplicationFn main, ApplicationFn resize, ApplicationFn teardown);


#endif // APPLICATION_H
