#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <libpad.h>
#include <sbv_patches.h>
#include <gsKit.h>
#include <dmaKit.h>
#include "global.h"
#include "virtuappu.h"

#ifdef GBA_H
#undef GBA_H
#endif

extern unsigned char usbd_irx[];
extern unsigned int size_usbd_irx;
extern unsigned char bdm_irx[];
extern unsigned int size_bdm_irx;
extern unsigned char bdmfs_fatfs_irx[];
extern unsigned int size_bdmfs_fatfs_irx;
extern unsigned char usbmass_bd_irx[];
extern unsigned int size_usbmass_bd_irx;
extern void AgbMain(void);
extern void PS2_SetLoadedRom(unsigned char *buf, size_t size);

static char *padBuf = NULL;
static GSGLOBAL *gsGlobal = NULL;
static GSTEXTURE gbaTex;
static u32 *ps2Frame32 = NULL;

#define PS2_GBA_W 240
#define PS2_GBA_H 160
#define PS2_GBA_PIXELS (PS2_GBA_W * PS2_GBA_H)

void init_drivers(void) {
    SifInitRpc(0);
    while (!SifIopReset("", 0)) {};
    while (!SifIopSync()) {};
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    SifExecModuleBuffer(usbd_irx, size_usbd_irx, 0, NULL, NULL);
    SifExecModuleBuffer(bdm_irx, size_bdm_irx, 0, NULL, NULL);
    SifExecModuleBuffer(bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, NULL);
    SifExecModuleBuffer(usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, NULL);
}

static void update_frame_texture_ptr(void) {
    size_t bufBytes = sizeof(virtuappu_frame_buffer);
    if (bufBytes <= PS2_GBA_PIXELS * 2 && ps2Frame32) {
        const u16 *src16 = (const u16 *)virtuappu_frame_buffer;
        for (int i = 0; i < PS2_GBA_PIXELS; i++) {
            u16 px = src16[i];
            u32 r = ((px & 0x001F) * 255) / 31;
            u32 g = (((px >> 5) & 0x001F) * 255) / 31;
            u32 b = (((px >> 10) & 0x001F) * 255) / 31;
            ps2Frame32[i] = r | (g << 8) | (b << 16) | (0x80U << 24);
        }
        gbaTex.Mem = ps2Frame32;
    } else {
        gbaTex.Mem = (u32 *)virtuappu_frame_buffer;
    }
}

void PS2_PresentFrameAndPollInput(void) {
    struct padButtonStatus buttons;
    u16 gba_keys = 0x03FF;
    int state = padGetState(0, 0);
    if (state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1) {
        if (padRead(0, 0, &buttons) != 0) {
            u32 pad = 0xffff ^ buttons.btns;
            if (pad & PAD_CROSS)  gba_keys &= ~(1 << 0);
            if (pad & PAD_CIRCLE) gba_keys &= ~(1 << 1);
            if (pad & PAD_SELECT) gba_keys &= ~(1 << 2);
            if (pad & PAD_START)  gba_keys &= ~(1 << 3);
            if (pad & PAD_RIGHT)  gba_keys &= ~(1 << 4);
            if (pad & PAD_LEFT)   gba_keys &= ~(1 << 5);
            if (pad & PAD_UP)     gba_keys &= ~(1 << 6);
            if (pad & PAD_DOWN)   gba_keys &= ~(1 << 7);
            if (pad & PAD_R1)     gba_keys &= ~(1 << 8);
            if (pad & PAD_L1)     gba_keys &= ~(1 << 9);
            if (buttons.ljoy_h < 60)  gba_keys &= ~(1 << 5);
            if (buttons.ljoy_h > 195) gba_keys &= ~(1 << 4);
            if (buttons.ljoy_v < 60)  gba_keys &= ~(1 << 6);
            if (buttons.ljoy_v > 195) gba_keys &= ~(1 << 7);
        }
    }
    *(vu16 *)(gIoMem + 0x130) = gba_keys;

    if (gsGlobal) {
        update_frame_texture_ptr();
        gsKit_texture_upload(gsGlobal, &gbaTex);
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x00));
        gsKit_prim_sprite_texture(gsGlobal, &gbaTex,
            0.0f, 0.0f, 0.0f, 0.0f,
            (float)gsGlobal->Width, (float)gsGlobal->Height,
            (float)PS2_GBA_W, (float)PS2_GBA_H,
            1, GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00));
        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    init_drivers();

    padBuf = (char *)memalign(64, 256);
    if (padBuf) {
        padInit(0);
        padPortOpen(0, 0, padBuf);
    }

    ps2Frame32 = (u32 *)memalign(64, PS2_GBA_PIXELS * sizeof(u32));
    if (ps2Frame32) {
        memset(ps2Frame32, 0, PS2_GBA_PIXELS * sizeof(u32));
    }

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsGlobal = gsKit_init_global();
    if (gsGlobal) {
        gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;
        gsKit_init_screen(gsGlobal);
        gsKit_mode_switch(gsGlobal, GS_ONESHOT);
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x00));
        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);

        memset(&gbaTex, 0, sizeof(GSTEXTURE));
        gbaTex.Width = PS2_GBA_W;
        gbaTex.Height = PS2_GBA_H;
        gbaTex.PSM = GS_PSM_CT32;
        gbaTex.Filter = GS_FILTER_NEAREST;
        u32 alloc = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(PS2_GBA_W, PS2_GBA_H, GS_PSM_CT32), GSKIT_ALLOC_USERBUFFER);
        gbaTex.Vram = (alloc != GSKIT_ALLOC_ERROR) ? alloc : 0;
    }

    memset(virtuappu_frame_buffer, 0, sizeof(virtuappu_frame_buffer));

    FILE *rom = NULL;
    for (int i = 0; i < 15; i++) {
        rom = fopen("mass:/minish.gba", "rb");
        if (rom != NULL) break;
        usleep(150000);
    }
    if (rom != NULL) {
        fseek(rom, 0, SEEK_END);
        long tamanho = ftell(rom);
        fseek(rom, 0, SEEK_SET);
        if (tamanho > 0 && tamanho <= 0x2000000) {
            unsigned char *rom_buffer = (unsigned char *)memalign(64, (size_t)tamanho);
            if (rom_buffer) {
                size_t lidos = fread(rom_buffer, 1, (size_t)tamanho, rom);
                if (lidos > 0) {
                    PS2_SetLoadedRom(rom_buffer, lidos);
                } else {
                    free(rom_buffer);
                }
            }
        }
        fclose(rom);
    }

    AgbMain();
    return 0;
}
