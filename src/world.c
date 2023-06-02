#include "world.h"

World* world_init(World* world, const char* name) {
    World* w = world ? world : NEW(World, 1);

    ground_init(&w->ground, 500);

    return w;
}

void world_destroy(World* world) {
    ground_destroy(&world->ground);
}