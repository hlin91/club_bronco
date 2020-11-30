# Makefile for Club Bronco
CC := clang++
CCWINDOWS := cl

.PHONY: all clean

all: client server

client: src/game_cli.cpp src/client.hpp src/client.cpp src/olcPixelGameEngine/olcPixelGameEngine.h src/olcPixelGameEngine/stb_image.h
	ifeq($(OS), Windows_NT)
		@${CCWINDOWS} -O2 -w -std:c++17 src/game_cli.cpp -o client
	else
		UNAME_S := $(shell uname -s)
		ifeq($(UNAME_S), Linux)
			@${CC} -O2 -w -o client src/game_cli.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
		endif
		ifeq($(UNAME_S), Darwin)
			@${CC} -O2 -w -o client src/game_cli.cpp -std=c++17 -macosx-version-min=10.15 -framework OpenGL -framework GLUT
		endif
	endif
src/game_cli.o: src/client.hpp src/olcPixelGameEngine/olcPixelGameEngine.h src/olcPixelGameEngine/stb_image.h
	ifeq($(OS), Windows_NT)
	@${CCWINDOWS} -c -O2 src/game_cli.cpp -o src/game_cli.o
	else
	@${CC} -c -O2 src/game_cli.cpp -o src/game_cli.o
	endif

server:  src/Parser.o src/server.o
	ifeq($(OS), Windows_NT)
	@${CCWINDOWS} -O2 -o server src/Parser.o src/server.o
	else
	@${CC} -O2 -o server src/Parser.o src/server.o
	endif
src/Parser.o: src/Parser.c src/Parser.h
	ifeq($(OS), Windows_NT)
	@${CCWINDOWS} -c -O2 src/Parser.c -o src/Parser.o
	else
	@${CC} -c -O2 src/Parser.c -o src/Parser.o
	endif

src/server.o: src/server.cpp
	ifeq($(OS), Windows_NT)
	@${CCWINDOWS} -c -O2 src/server.cpp -o src/server.o
	else
	@${CC} -c -O2 src/server.cpp -o src/server.o
	endif

clean:
	@rm ./src/*.o

