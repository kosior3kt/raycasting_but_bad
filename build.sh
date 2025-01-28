#!/bin/bash

gcc src/main.c src/game.c -g -o DO_NOT_RESIZE \
	-I/Users/jk/Programming/LIBRARIES/SDL/include/SDL3 \
	-L/Users/jk/Programming/LIBRARIES/SDL/build \
	-lSDL3 \
	-rpath /Users/jk/Programming/LIBRARIES/SDL/build
