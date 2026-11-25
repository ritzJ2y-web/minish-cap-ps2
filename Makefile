EE_BIN = MINISH_PS2.ELF
EE_OBJS = main_ps2.o
EE_LIBS = -ldebug

all: $(EE_BIN)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
