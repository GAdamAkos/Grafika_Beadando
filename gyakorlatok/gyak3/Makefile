CC = C:\c_sdk_220203\MinGW\bin\gcc.exe
CFLAGS = -Wall -Wextra -std=c11 -IC:\c_sdk_220203\MinGW\include
LDFLAGS = -LC:\c_sdk_220203\MinGW\lib -lmingw32 -lSDL2main -lSDL2
MATHFLAGS = -lm

LINE_TARGET = line_rect_editor.exe
LINE_SRC = line_rect_editor.c

CIRCLE_TARGET = circle_editor.exe
CIRCLE_SRC = circle_editor.c

all: $(LINE_TARGET) $(CIRCLE_TARGET)

$(LINE_TARGET): $(LINE_SRC)
	$(CC) $(CFLAGS) $(LINE_SRC) -o $(LINE_TARGET) $(LDFLAGS)

$(CIRCLE_TARGET): $(CIRCLE_SRC)
	$(CC) $(CFLAGS) $(CIRCLE_SRC) -o $(CIRCLE_TARGET) $(LDFLAGS) $(MATHFLAGS)

run-line: $(LINE_TARGET)
	$(LINE_TARGET)

run-circle: $(CIRCLE_TARGET)
	$(CIRCLE_TARGET)

clean:
	del /Q $(LINE_TARGET) 2>nul
	del /Q $(CIRCLE_TARGET) 2>nul

.PHONY: all run-line run-circle clean