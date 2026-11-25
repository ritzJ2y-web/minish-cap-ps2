#include <stdio.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <debug.h>

int main(int argc, char *argv[]) {
    SifInitRpc(0);
    init_scr();

    scr_printf("\n\n========================================\n");
    scr_printf("   MINISH CAP PS2 - AMBIENTE PRONTO!    \n");
    scr_printf("========================================\n\n");
    scr_printf("Compilado na nuvem via GitHub Actions.\n");
    scr_printf("Proximo passo: carregar ROM e video.\n");

    SleepThread();
    return 0;
}
