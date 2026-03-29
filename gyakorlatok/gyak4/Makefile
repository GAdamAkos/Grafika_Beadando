CC = C:\c_sdk_220203\MinGW\bin\gcc.exe
CFLAGS = -Wall -Wextra -std=c11 -IC:\c_sdk_220203\MinGW\include
LDFLAGS = -LC:\c_sdk_220203\MinGW\lib -lmingw32 -lSDL2main -lSDL2 -lm

TARGET = pong_game.exe
SRC = pong_game.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	.\$(TARGET)

clean:
	del /Q $(TARGET) 2>nul

.PHONY: all run clean