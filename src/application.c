#include "application.h"

Application* application_init(Application* a, void* owner, ApplicationFn setup, ApplicationFn main, ApplicationFn resize, ApplicationFn teardown) {
    Application* application = a ? a : NEW(Application, 1);

    application->setup = setup;
    application->main = main;
    application->resize = resize;
    application->teardown = teardown;

    application->owner = owner;

    return application;
}