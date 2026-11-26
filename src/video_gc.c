#include <gccore.h>
#include <ogc/gu.h>
#include "video_gc.h"

#define GX_FIFO_SIZE (256 * 1024)

/* GX requires an aligned, dedicated FIFO. Do not use an XFB as the FIFO. */
static u8 s_fifo[GX_FIFO_SIZE] __attribute__((aligned(32)));
static GXRModeObj *s_mode;
static void *s_xfb[2];
static unsigned s_back_buffer = 1;

void video_gc_init(void)
{
    Mtx44 projection;
    Mtx modelview;
    f32 yscale;
    u32 xfb_height;
    u32 margin_x, margin_y, play_width, play_height;
    const GXColor clear_color = { 0, 0, 0, 255 };

    VIDEO_Init();
    s_mode = VIDEO_GetPreferredMode(NULL);
    s_xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(s_mode));
    s_xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(s_mode));

    VIDEO_Configure(s_mode);
    VIDEO_SetNextFramebuffer(s_xfb[0]);
    VIDEO_SetBlack(GX_TRUE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (s_mode->viTVMode & VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }

    GX_Init(s_fifo, sizeof(s_fifo));
    GX_SetCopyClear(clear_color, GX_MAX_Z24);

    /* Inset the entire logical world by 5% on each edge of the active mode.
       The matching scissor clips wrapped geometry at the visible boundary.
       Keep the display copy full-sized so the margins stay black. */
    margin_x = s_mode->fbWidth / 20u;
    margin_y = s_mode->efbHeight / 20u;
    play_width = s_mode->fbWidth - 2u * margin_x;
    play_height = s_mode->efbHeight - 2u * margin_y;
    GX_SetViewport((f32)margin_x, (f32)margin_y, (f32)play_width,
                   (f32)play_height, 0.0f, 1.0f);
    GX_SetScissor(margin_x, margin_y, play_width, play_height);
    yscale = GX_GetYScaleFactor(s_mode->efbHeight, s_mode->xfbHeight);
    xfb_height = GX_SetDispCopyYScale(yscale);
    GX_SetDispCopySrc(0, 0, s_mode->fbWidth, s_mode->efbHeight);
    GX_SetDispCopyDst(s_mode->fbWidth, xfb_height);
    GX_SetCopyFilter(s_mode->aa, s_mode->sample_pattern, GX_TRUE, s_mode->vfilter);
    GX_SetFieldMode(s_mode->field_rendering,
                    (s_mode->viHeight == 2 * s_mode->xfbHeight) ? GX_ENABLE : GX_DISABLE);
    GX_SetPixelFmt(s_mode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetDispCopyGamma(GX_GM_1_0);

    /* The game always uses 640x480 logical coordinates, top-left origin. */
    guOrtho(projection, 0.0f, GAME_HEIGHT, 0.0f, GAME_WIDTH, 0.0f, 300.0f);
    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);
    guMtxIdentity(modelview);
    GX_LoadPosMtxImm(modelview, GX_PNMTX0);
    GX_SetCurrentMtx(GX_PNMTX0);

    GX_SetCullMode(GX_CULL_NONE);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    GX_SetColorUpdate(GX_TRUE);

    /* Position + unlit per-vertex color; no textures needed in Phase 1. */
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                   GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
    GX_SetNumTexGens(0);
    GX_SetNumTevStages(1);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    /* Prime the EFB to black; later copies clear it after each presented frame. */
    GX_CopyDisp(s_xfb[0], GX_TRUE);
    GX_DrawDone();
}

void video_gc_present(void)
{
    /* The current frame's vertices have already been submitted to GX. */
    GX_CopyDisp(s_xfb[s_back_buffer], GX_TRUE);
    GX_DrawDone();

    VIDEO_SetNextFramebuffer(s_xfb[s_back_buffer]);
    VIDEO_SetBlack(GX_FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    s_back_buffer ^= 1;
}
