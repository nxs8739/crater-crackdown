#include <gccore.h>
#include "input.h"

void input_init(void)
{
    PAD_Init();
}

void input_scan(void)
{
    PAD_ScanPads();
}

u32 input_pressed(void)
{
    return PAD_ButtonsDown(0);
}

u32 input_held(void)
{
    return PAD_ButtonsHeld(0);
}
