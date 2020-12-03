# Makefile for Club Bronco
CC := clang++
UN=$(shell uname -s)

.PHONY: all clean

all: client server

client: src/game_cli.cpp src/client.hpp src/Parser.o src/client.cpp src/olcPixelGameEngine/olcPixelGameEngine.h src/olcPixelGameEngine/stb_image.h
ifeq ($(UN),Linux)
	@${CC} -O2 -w -o client src/game_cli.cpp src/Parser.o -lX11 -lGL -lpthread -lstdc++fs -std=c++17 -g
endif
ifeq ($(UN),Darwin)
	@${CC} -O2 -w -o client src/game_cli.cpp src/Parser.o -std=c++17 -mmacosx-version-min=10.15 -framework OpenGL -framework GLUT
endif

server: src/Parser.o src/server.o
	@${CC} -O2 -std=c++17 -o server src/Parser.o src/server.o -lpthread

src/Parser.o: src/Parser.cpp src/Parser.h
	@${CC} -c -O2 -std=c++17 src/Parser.cpp -o src/Parser.o

src/server.o: src/server.cpp
	@${CC} -c -O2 -std=c++17 src/server.cpp -o src/server.o

clean:
	@rm ./src/*.o
