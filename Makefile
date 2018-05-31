all: main.c yixin.h
	gcc -Wall -O2 main.c -o Yixin -lm `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs --cflags cairo`
