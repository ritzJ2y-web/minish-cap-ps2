EE_BIN = MINISH_PS2.ELF
EE_OBJS = main_ps2.o usbd_irx.o bdm_irx.o bdmfs_fatfs_irx.o usbmass_bd_irx.o
EE_INCS = -I$(PS2DEV)/gsKit/include
EE_LDFLAGS = -L$(PS2DEV)/gsKit/lib
EE_LIBS = -lgskit -ldmakit -ldebug -lpad -lpatches

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
