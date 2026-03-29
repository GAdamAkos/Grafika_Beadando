CC = C:\c_sdk_220203\MinGW\bin\gcc.exe
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lm
TARGET = matrix_demo.exe
SRC = matrix_demo.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	$(TARGET)

clean:
	del /Q $(TARGET) 2>nul

.PHONY: all run clean