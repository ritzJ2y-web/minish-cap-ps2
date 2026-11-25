#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

extern unsigned char usbd_irx[];
extern unsigned int size_usbd_irx;
extern unsigned char bdm_irx[];
extern unsigned int size_bdm_irx;
extern unsigned char bdmfs_fatfs_irx[];
extern unsigned int size_bdmfs_fatfs_irx;
extern unsigned char usbmass_bd_irx[];
extern unsigned int size_usbmass_bd_irx;

static char padBuf[256] __attribute__((aligned(64)));

// Framebuffer virtual do GBA (240x160 pixels em 16-bit RGB555)
#define GBA_W 240
#define GBA_H 160
static u16 gba_framebuffer[GBA_W * GBA_H] __attribute__((aligned(64)));

void init_drivers() {
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

int main(int argc, char *argv[]) {
    init_drivers();

    padInit(0);
    padPortOpen(0, 0, padBuf);

    // Carrega a ROM de 16MB para a RAM
    FILE *rom = NULL;
    unsigned char *rom_buffer = NULL;
    for (int i = 0; i < 25; i++) {
        rom = fopen("mass:/minish.gba", "rb");
        if (rom != NULL) break;
        usleep(200000);
    }

    if (rom != NULL) {
        fseek(rom, 0, SEEK_END);
        long tamanho = ftell(rom);
        fseek(rom, 0, SEEK_SET);
        rom_buffer = (unsigned char *)malloc(tamanho);
        if (rom_buffer) fread(rom_buffer, 1, tamanho, rom);
        fclose(rom);
    }

    // Inicializa o motor grafico real do PS2 (gsKit)
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    GSGLOBAL *gsGlobal = gsKit_init_global();
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;
    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

    // Configura a textura de 240x160 que vai receber os quadros do jogo
    GSTEXTURE gbaTex;
    memset(&gbaTex, 0, sizeof(GSTEXTURE));
    gbaTex.Width = GBA_W;
    gbaTex.Height = GBA_H;
    gbaTex.PSM = GS_PSM_CT16; // Formato 16-bit identico ao do GBA!
    gbaTex.Mem = (u32 *)gba_framebuffer;
    gbaTex.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(GBA_W, GBA_H, GS_PSM_CT16), GSKIT_ALLOC_USERBUFFER);
    gbaTex.Filter = GS_FILTER_NEAREST; // Pixel art nitido

    struct padButtonStatus buttons;
    u32 paddata = 0;
    int box_x = 100, box_y = 60;
    u16 cor_fundo = rom_buffer ? 0x2945 : 0x001F; // Verde se a ROM carregou, Vermelho se falhou

    while (1) {
        if (padRead(0, 0, &buttons) != 0) {
            paddata = 0xffff ^ buttons.btns;
            if (paddata & PAD_LEFT && box_x > 0) box_x -= 2;
            if (paddata & PAD_RIGHT && box_x < GBA_W - 20) box_x += 2;
            if (paddata & PAD_UP && box_y > 0) box_y -= 2;
            if (paddata & PAD_DOWN && box_y < GBA_H - 20) box_y += 2;
            if (paddata & PAD_CROSS) cor_fundo = 0x7C00; // Azul ao apertar X
            if (paddata & PAD_CIRCLE) cor_fundo = 0x03E0; // Verde vivo ao apertar O
        }

        // Desenha no framebuffer 240x160 (exatamente como o jogo fara)
        for (int y = 0; y < GBA_H; y++) {
            for (int x = 0; x < GBA_W; x++) {
                if (x >= box_x && x < box_x + 20 && y >= box_y && y < box_y + 20) {
                    gba_framebuffer[y * GBA_W + x] = 0x7FFF; // Quadrado branco (Link)
                } else {
                    gba_framebuffer[y * GBA_W + x] = cor_fundo;
                }
            }
        }

        // Envia o quadro 240x160 para a placa de video do PS2 e estica na TV
        gsKit_texture_upload(gsGlobal, &gbaTex);
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x00));
        gsKit_prim_sprite_texture(gsGlobal, &gbaTex,
            0.0f, 0.0f, 0.0f, 0.0f,
            gsGlobal->Width, gsGlobal->Height, GBA_W, GBA_H,
            1, GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00));

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
