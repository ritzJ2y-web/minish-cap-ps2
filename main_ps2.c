#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <debug.h>
#include <libpad.h>
#include <sbv_patches.h>

static char padBuf[256] __attribute__((aligned(64)));

void init_drivers() {
    SifInitRpc(0);
    while (!SifIopReset("", 0)) {};
    while (!SifIopSync()) {};
    SifInitRpc(0);

    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    // Carrega modulos essenciais de controle e USB (pendrive)
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    SifLoadModule("rom0:USBHDFSD", 0, NULL);
}

int main(int argc, char *argv[]) {
    init_drivers();
    init_scr();

    scr_printf("\n========================================\n");
    scr_printf("   MINISH CAP PS2 - TESTE DE ROM E PAD  \n");
    scr_printf("========================================\n\n");

    // Inicializa o controle na Porta 1
    padInit(0);
    padPortOpen(0, 0, padBuf);
    scr_printf("[OK] Driver de controle inicializado.\n");

    // Aguarda 2 segundos para o pendrive montar
    scr_printf("[..] Procurando mass:/minish.gba ...\n");
    for (int i = 0; i < 120; i++) {
        // Espera simples
        for (volatile int j = 0; j < 500000; j++);
    }

    FILE *rom = fopen("mass:/minish.gba", "rb");
    if (rom != NULL) {
        fseek(rom, 0, SEEK_END);
        long tamanho = ftell(rom);
        fseek(rom, 0, SEEK_SET);

        unsigned char *rom_buffer = (unsigned char *)malloc(tamanho);
        if (rom_buffer != NULL) {
            fread(rom_buffer, 1, tamanho, rom);
            scr_printf("[SUCESSO!] ROM carregada na RAM!\n");
            scr_printf("Tamanho: %ld bytes (%.2f MB)\n", tamanho, tamanho / (1024.0 * 1024.0));
            scr_printf("Cabecalho GBA: %.12s\n\n", &rom_buffer[0xA0]);
            free(rom_buffer);
        } else {
            scr_printf("[ERRO] Memoria RAM insuficiente!\n");
        }
        fclose(rom);
    } else {
        scr_printf("[AVISO] Arquivo mass:/minish.gba nao encontrado.\n");
        scr_printf("Coloque a ROM na raiz do pendrive.\n\n");
    }

    scr_printf("Pressione X ou START no DualShock 2...\n");

    struct padButtonStatus buttons;
    u32 paddata, old_pad = 0, new_pad;

    while (1) {
        int state = padGetState(0, 0);
        if (state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1) {
            if (padRead(0, 0, &buttons) != 0) {
                paddata = 0xffff ^ buttons.btns;
                new_pad = paddata & ~old_pad;
                old_pad = paddata;

                if (new_pad & PAD_CROSS) {
                    scr_printf("Botao [X] (A do GBA) detectado!\n");
                }
                if (new_pad & PAD_CIRCLE) {
                    scr_printf("Botao [O] (B do GBA) detectado!\n");
                }
                if (new_pad & PAD_START) {
                    scr_printf("Botao [START] detectado!\n");
                }
            }
        }
    }

    return 0;
}
