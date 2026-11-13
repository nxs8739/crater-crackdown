/* Minimal host-only ASND mock for testing game audio event dispatch.
 * Not included by a GameCube build; the real libogc header is used there.
 */
#ifndef TEST_ASNDLIB_H
#define TEST_ASNDLIB_H
#include <stdint.h>
typedef int32_t s32;
#define SND_OK 0
#define SND_UNUSED 0
#define SND_WORKING 1
#define VOICE_MONO_16BIT 1
void ASND_Init(void);
void ASND_End(void);
void ASND_Pause(s32 pause);
s32 ASND_SetVoice(s32 voice, s32 format, s32 pitch, s32 delay,
                  void *snd, s32 size, s32 left, s32 right,
                  void (*callback)(s32 voice));
s32 ASND_StopVoice(s32 voice);
s32 ASND_StatusVoice(s32 voice);
#endif
