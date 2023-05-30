#ifndef BLOCK_INTERNAL_H
#define BLOCK_INTERNAL_H

#include "../block.h"

#define BLOCK_ACTIVE_BIT        9
#define BLOCK_COLOR_MASK        ((1 << BLOCK_ACTIVE_BIT) - 1)
#define BLOCK_ACTIVE_MASK       (1 << BLOCK_ACTIVE_BIT)
#define BLOCK_ACTIVE(x)         (x & BLOCK_ACTIVE_MASK)
#define BLOCK_COLOR(x)          (x & BLOCK_COLOR_MASK)

#endif // BLOCK_INTERNAL_H
