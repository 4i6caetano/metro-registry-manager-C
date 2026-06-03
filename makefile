INCLUDES = -I./utils -I./header -I./registry -I./functions -I./indexArchive

all:
	gcc $(INCLUDES) -o programaTrab main.c utils/*.c header/*.c registry/*.c functions/*.c

run:
	./programaTrab