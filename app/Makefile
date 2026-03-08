APP_NAME = substation

# Prefer the course C SDK gcc if present
SDK_GCC = C:/c_sdk_220203/MinGW/bin/gcc.exe
ifeq ("$(wildcard $(SDK_GCC))","")
CC = gcc
else
CC = $(SDK_GCC)
endif

CFLAGS = -Wall -Wextra -Wpedantic -std=c99 \
	-Iinclude \
	-Iext/obj/include

LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglu32 -lm

SRC = src/main.c src/camera.c src/help.c src/texture.c
OBJ_SRC = ext/obj/src/model.c ext/obj/src/load.c ext/obj/src/info.c ext/obj/src/draw.c ext/obj/src/transform.c

all:
	$(CC) $(CFLAGS) $(SRC) $(OBJ_SRC) $(LDFLAGS) -o $(APP_NAME).exe

clean:
	del /q *.exe 2>nul || exit 0