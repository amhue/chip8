CC = gcc
PPCFLAGS = -I./SDL2/include

ifeq ($(OS),Windows_NT)
	FILES = ./build/main.o ./build/chip8.o ./build/icon.o
	EXEC = chip8.exe
	MKBUILD = cmd /c "mkdir build"
	RMBUILD = cmd /c "rmdir /s /q build"
	RM = cmd /c "del chip8.exe"
	LDFLAGS = -L./SDL2/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
else
	FILES = ./build/main.o ./build/chip8.o
	EXEC = chip8
	MKBUILD = if [ ! -d "./build" ]; then mkdir build; fi
	RMBUILD = rm -rf build
	RM = rm chip8
	LDFLAGS = -L./SDL2/lib -lSDL2 -lSDL2main
endif

all: $(FILES)
	$(CC) $(FILES) -o $(EXEC) $(LDFLAGS)

./build/main.o: build
	$(CC) -c ./src/main.c -o ./build/main.o $(PPCFLAGS)

./build/chip8.o: build
	$(CC) -c ./src/chip8.c -o ./build/chip8.o

./build/icon.o: build
	windres -i ./icon.rc -o ./build/icon.o

build:
	$(MKBUILD)

clean:
	$(RMBUILD)
	$(RM)
