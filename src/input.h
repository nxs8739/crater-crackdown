#ifndef BEANZ_INPUT_H
#define BEANZ_INPUT_H

#include <gccore.h>

void input_init(void);
void input_scan(void);
u32 input_pressed(void);
u32 input_held(void);

#endif
