SDK_GCC = C:/c_sdk_220203/MinGW/bin/gcc.exe
SDK_AR  = C:/c_sdk_220203/MinGW/bin/ar.exe

ifeq ("$(wildcard $(SDK_GCC))","")
CC = gcc
else
CC = $(SDK_GCC)
endif

ifeq ("$(wildcard $(SDK_AR))","")
AR = ar
else
AR = $(SDK_AR)
endif

all:
	$(CC) -Wall -Wextra -Wpedantic -std=c99 -c ./src/model.c -o model.o
	$(CC) -Wall -Wextra -Wpedantic -std=c99 -c ./src/load.c -o load.o
	$(CC) -Wall -Wextra -Wpedantic -std=c99 -c ./src/info.c -o info.o
	$(CC) -Wall -Wextra -Wpedantic -std=c99 -c ./src/draw.c -o draw.o
	$(CC) -Wall -Wextra -Wpedantic -std=c99 -c ./src/transform.c -o transform.o
	$(AR) rcs libobj.a model.o load.o info.o draw.o transform.o

clean:
	del /q model.o 2>nul || exit 0
	del /q load.o 2>nul || exit 0
	del /q info.o 2>nul || exit 0
	del /q draw.o 2>nul || exit 0
	del /q transform.o 2>nul || exit 0
	del /q libobj.a 2>nul || exit 0