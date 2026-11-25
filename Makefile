EE_BIN = MINISH_PS2.ELF

GAME_SRCS = $(filter-out picori_src/src/eeprom.c, $(wildcard picori_src/src/*.c)) \
            $(filter-out picori_src/src/gba/%, $(wildcard picori_src/src/*/*.c))

PORT_C_SRCS = $(filter-out picori_src/port/port_main.c picori_src/port/port_audio.c picori_src/port/port_bios.c picori_src/port/port_softslots.c picori_src/port/port_ra*.c picori_src/port/port_gba_shadow.c picori_src/port/port_discord_rpc.c picori_src/port/port_repro*.c picori_src/port/port_debug_actions_test.c picori_src/port/port_memory_watch_test.c picori_src/port/port_rng_golden_test.c picori_src/port/port_gpu_obj_cull.c picori_src/port/port_a11y*.c picori_src/port/port_quicksave.c picori_src/port/port_practice.c picori_src/port/port_update_check.c picori_src/port/port_rom_picker.c picori_src/port/port_upscale.c picori_src/port/port_filter.c picori_src/port/port_shm_framebuffer.c picori_src/port/port_menu_background.c picori_src/port/port_bugreport_state.c picori_src/port/port_debug_actions.c picori_src/port/port_debug_entities.c picori_src/port/port_debug_memory_watch.c, $(wildcard picori_src/port/*.c)) \
              picori_src/port/ppu/src/virtuappu.c \
              picori_src/port/ppu/src/mode1.c

ALL_C_SRCS = main_ps2.c ps2_bridge.c ps2_audio_stub.c $(GAME_SRCS) $(PORT_C_SRCS)
EE_OBJS = $(ALL_C_SRCS:.c=.o) usbd_irx.o bdm_irx.o bdmfs_fatfs_irx.o usbmass_bd_irx.o

EE_INCS = -I$(PS2DEV)/gsKit/include -Ips2_shims -Ipicori_src/include -Ipicori_src/port -Ipicori_src/port/ppu/include -Ipicori_src/build/USA -Ipicori_src
EE_CFLAGS = -std=gnu11 -D"_Static_assert(...)=" -DTMC_N64 -DPC_PORT -DNON_MATCHING -DUSE_HDMA -DUSA -DENGLISH -DREVISION=0 -DMODE1_GBA_WIDTH=240 -funsigned-char -fno-strict-aliasing -fwrapv -O2 -Wno-implicit-function-declaration -Wno-incompatible-pointer-types
EE_LDFLAGS = -L$(PS2DEV)/gsKit/lib
EE_LIBS = -lgskit -ldmakit -lpad -lpatches -lm

all: $(EE_BIN)

usbd_irx.c:
	bin2c $(PS2SDK)/iop/irx/usbd.irx usbd_irx.c usbd_irx

bdm_irx.c:
	bin2c $(PS2SDK)/iop/irx/bdm.irx bdm_irx.c bdm_irx

bdmfs_fatfs_irx.c:
	bin2c $(PS2SDK)/iop/irx/bdmfs_fatfs.irx bdmfs_fatfs_irx.c bdmfs_fatfs_irx

usbmass_bd_irx.c:
	bin2c $(PS2SDK)/iop/irx/usbmass_bd.irx usbmass_bd_irx.c usbmass_bd_irx

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
