#ifndef BEANZ_VIDEO_GC_H
#define BEANZ_VIDEO_GC_H

#define GAME_WIDTH 640.0f
#define GAME_HEIGHT 480.0f

/* Configure video and GX once. */
void video_gc_init(void);

/* Copy the just-drawn GX EFB to the next XFB, present, and VSync. */
void video_gc_present(void);

#endif
