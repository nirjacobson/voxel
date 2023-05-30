#include "block.h"
#include "internal/block.h"

char block_is_active(Block* block) {
  return BLOCK_ACTIVE(block->data) != 0;
}

uint16_t block_color(Block* block) {
  return BLOCK_COLOR(block->data);
}

void block_color_rgb(uint16_t color, float* components) {
  components[0] = BLOCK_COLOR_RED(color) * 255.0 / 7;
  components[1] = BLOCK_COLOR_GREEN(color) * 255.0 / 7;
  components[2] = BLOCK_COLOR_BLUE(color) * 255.0 / 7;
}

void block_set_active(Block* block, char active) {
  if (active) {
    block->data |= BLOCK_ACTIVE_MASK;
  } else {
    block->data &= ~BLOCK_ACTIVE_MASK;
  }
}

void block_set_color(Block* block, uint16_t color) {
  block->data &= ~BLOCK_COLOR_MASK;
  block->data |= color;
}
