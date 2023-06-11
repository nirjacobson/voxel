#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_COLOR_MASK_RED        (((1 << 3) - 1) << 0)
#define BLOCK_COLOR_MASK_GREEN      (((1 << 3) - 1) << 3)
#define BLOCK_COLOR_MASK_BLUE       (((1 << 3) - 1) << 6)
#define BLOCK_COLOR_RED(x)          ((x & BLOCK_COLOR_MASK_RED) >> 0)
#define BLOCK_COLOR_SET_RED(x, y)     x = ((x & ~BLOCK_COLOR_MASK_RED) | ((y) << 0))
#define BLOCK_COLOR_GREEN(x)        ((x & BLOCK_COLOR_MASK_GREEN) >> 3)
#define BLOCK_COLOR_SET_GREEN(x, y)   x = ((x & ~BLOCK_COLOR_MASK_GREEN) | ((y) << 3))
#define BLOCK_COLOR_BLUE(x)         ((x & BLOCK_COLOR_MASK_BLUE) >> 6)
#define BLOCK_COLOR_SET_BLUE(x, y)    x = ((x & ~BLOCK_COLOR_MASK_BLUE) | ((y) << 6))

#include <stdint.h>

#include "global.h"

typedef struct {
    uint16_t data;
} Block;

char block_is_active(Block* block);
uint16_t block_color(Block* block);
void block_color_rgb(uint16_t color, float* components);

void block_set_active(Block* block, char active);
void block_set_color(Block* block, uint16_t color);

#endif // BLOCK_H
