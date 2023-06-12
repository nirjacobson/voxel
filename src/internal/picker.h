#ifndef PICKER_INTERNAL_H
#define PICKER_INTERNAL_H

#include "../picker.h"

void picker_act(Picker* picker, char modifier1, char modifier2);
Box picker_merge_selections(Box* selectionA, Box* selectionB);

#endif // PICKER_INTERNAL_H
