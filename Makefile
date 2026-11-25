EE_BIN = MINISH_PS2.ELF

GAME_SRCS = $(filter-out picori_src/src/eeprom.c, $(wildcard picori_src/src/*.c)) \
            $(wildcard picori_src/src/playerItem/*.c) \
            $(wildcard picori_src/src/enemy/*.c) \
            $(wildcard picori_src/src/npc/*.c) \
            $(wildcard picori_src/src/object/*.c) \
            $(wildcard picori_src/src/projectile/*.c) \
            $(wildcard picori_src/src/manager/*.c) \
            $(wildcard picori_src/src/item/*.c) \
            $(wildcard picori_src/src/subtask/*.c) \
            $(wildcard picori_src/src/menu/*.c) \
            $(wildcard picori_src/src/worldEvent/*.c) \
            picori_src/src/data/figurineMenuData.c \
            picori_src/src/data/hitbox.c \
            picori_src/src/data/areaMetadata.c \
            picori_src/src/data/caveBorderMapData.c \
            picori_src/src/data/data_080046A4.c \
            picori_src/src/data/mapActTileToSurfaceType.c \
            picori_src/src/data/objPalettes.c \
            picori_src/src/data/screenTransitions.c \
            picori_src/src/data/transitions.c

PORT_C_SRCS = picori_src/port/ppu/src/virtuappu.c \
              picori_src/port/ppu/src/mode1.c \
              picori_src/port/port_rom.c \
              picori_src/port/port_gba_mem.c \
              picori_src/port/port_hdma.c \
              picori_src/port/port_save.c \
              picori_src/port/port_animation.c \
              picori_src/port/port_math.c \
              picori_src/port/port_text_render.c \
              picori_src/port/port_gameplay_stubs.c \
              picori_src/port/port_m4a_stubs.c \
              picori_src/port/port_room_funcs.c \
              picori_src/port/port_script_funcs.c \
              picori_src/port/port_draw.c \
              picori_src/port/port_stubs.c \
              picori_src/port/stubs_autogen.c \
              picori_src/port/data_stubs_autogen.c \
              picori_src/port/port_rom_stubs.c \
              picori_src/port/port_region_data.c \
              picori_src/port/flag_remap_generated.c \
              picori_src/port/port_inline_ptrs.c \
              picori_src/port/port_script_addrs.c \
              picori_src/port/port_offset_remap.c \
              picori_src/port/port_figurines.c \
              picori_src/port/port_softslots.c \
              picori_src/port/port_roll_attack_macro.c \
              picori_src/port/port_analog_movement.c

ALL_C_SRCS = main_ps2.c $(GAME_SRCS) $(PORT_C_SRCS)
EE_OBJS = $(ALL_C_SRCS:.c=.o) usbd_irx.o bdm_irx.o bdmfs_fatfs_irx.o usbmass_bd_irx.o

EE_INCS = -I$(PS2DEV)/gsKit/include -Ips2_shims -Ipicori_src/include -Ipicori_src/port -Ipicori_src/port/ppu/include -Ipicori_src/build/USA -Ipicori_src
EE_CFLAGS = -std=gnu11 -DTMC_N64 -DPC_PORT -DNON_MATCHING -DUSE_HDMA -DUSA -DENGLISH -DREVISION=0 -DMODE1_GBA_WIDTH=240 -funsigned-char -fno-strict-aliasing -fwrapv -O2 -Wno-implicit-function-declaration -Wno-incompatible-pointer-types
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
