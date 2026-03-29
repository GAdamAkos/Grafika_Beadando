CC = C:\c_sdk_220203\MinGW\bin\gcc.exe
CFLAGS = -Wall -Wextra -std=c11
TARGET = teglatest.exe
SRC = teglatest.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	$(TARGET)

clean:
	del /Q $(TARGET) 2>nul

.PHONY: all run clean