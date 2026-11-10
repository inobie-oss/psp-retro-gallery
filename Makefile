TARGET = RetroGallery
OBJS = main.o

CFLAGS = -O2 -G0 -Wall $(shell psp-pkg-config --cflags sdl2 SDL2_image)
LIBS = $(shell psp-pkg-config --libs sdl2 SDL2_image) -lpng -ljpeg -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Retro Gallery

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
