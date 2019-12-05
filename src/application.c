#include "application.h"

Application* application_init(Application* a, struct Voxel* voxel, ApplicationFn setup, ApplicationFn main, ApplicationFn resize, ApplicationFn teardown) {
    Application* application = a ? a : NEW(Application, 1);

    application->setup = setup;
    application->main = main;
    application->resize = resize;
    application->teardown = teardown;

    application->voxel = voxel;

    return application;
}