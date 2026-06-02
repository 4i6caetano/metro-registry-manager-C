INCLUDES = -I./utils -I./header -I./registry -I./functions

all:
	gcc -o programaTrab *.c

run:
	./programaTrab