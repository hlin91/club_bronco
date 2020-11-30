# Makefile for Club Bronco
CC := clang++

.PHONY: all clean

all: client server

client: src/game_cli.cpp src/client.hpp src/client.cpp src/olcPixelGameEngine/olcPixelGameEngine.h src/olcPixelGameEngine/stb_image.h
	UNAME_S := $(shell uname -s)
	ifeq($(UNAME_S), Linux)
		@${CC} -O2 -w -o client src/game_cli.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	endif
	ifeq($(UNAME_S), Darwin)
		@${CC} -O2 -w -o client src/game_cli.cpp -std=c++17 -macosx-version-min=10.15 -framework OpenGL -framework GLUT
	endif

server:  src/Parser.o src/server.o
	@${CC} -O2 -o server src/Parser.o src/server.o

src/Parser.o: src/Parser.c src/Parser.h
	@${CC} -c -O2 src/Parser.c -o src/Parser.o

src/server.o: src/server.cpp
	@${CC} -c -O2 src/server.cpp -o src/server.o

clean:
	@rm ./src/*.o
