APP_NAME = substation

SDK_GCC = C:/c_sdk_220203/MinGW/bin/gcc.exe

ifeq ("$(wildcard $(SDK_GCC))","")
CC = gcc
else
CC = $(SDK_GCC)
endif

CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -Iinclude

SRC = \
	src/main.c \
	src/camera.c \
	src/help.c \
	src/texture.c \
	src/scene.c

OBJLIB = ext/obj/libobj.a

LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglu32 -lm

all: $(OBJLIB)
	$(CC) $(CFLAGS) $(SRC) $(OBJLIB) $(LDFLAGS) -o $(APP_NAME).exe

$(OBJLIB):
	$(MAKE) -C ext/obj

clean:
	del /q $(APP_NAME).exe 2>nul || exit 0
	$(MAKE) -C ext/obj clean