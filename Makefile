CC = gcc
PPCFLAGS = -I./SDL2/include

ifeq ($(OS),Windows_NT)
	FILES = ./build/main.o ./build/chip8.o ./build/icon.o ./build/audio.o ./build/display.o ./build/timer.o
	EXEC = chip8.exe
	MKBUILD = cmd /c "mkdir build"
	RMBUILD = cmd /c "rmdir /s /q build"
	RM = cmd /c "del chip8.exe"
	LDFLAGS = -L./SDL2/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
else
	FILES = ./build/main.o ./build/chip8.o ./build/audio.o ./build/display.o ./build/timer.o
	EXEC = chip8
	MKBUILD = if [ ! -d "./build" ]; then mkdir build; fi
	RMBUILD = rm -rf build
	RM = rm chip8
	LDFLAGS = -L./SDL2/lib -lSDL2main -lSDL2
endif

all: $(FILES)
	$(CC) $(FILES) -o $(EXEC) $(LDFLAGS)

./build/main.o: build ./src/main.c
	$(CC) -c ./src/main.c -o ./build/main.o $(PPCFLAGS)

./build/audio.o: build ./src/audio.c
	$(CC) -c ./src/audio.c -o ./build/audio.o $(PPCFLAGS)

./build/display.o: build ./src/display.c
	$(CC) -c ./src/display.c -o ./build/display.o $(PPCFLAGS)

./build/chip8.o: build ./src/chip8.c
	$(CC) -c ./src/chip8.c -o ./build/chip8.o

./build/icon.o: build ./icon.rc
	windres -i ./icon.rc -o ./build/icon.o

./build/timer.o: build ./src/timer.c
	$(CC) -c ./src/timer.c -o ./build/timer.o

build:
	$(MKBUILD)

clean:
	$(RMBUILD)
	$(RM)

.PHONY: all clean
