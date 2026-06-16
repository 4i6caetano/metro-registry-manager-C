INCLUDES = -I./utils -I./header -I./registry -I./functions -I./indexArchive

all:
	gcc $(INCLUDES) -o programaTrab main.c fornecidas.c utils/*.c header/*.c registry/*.c functions/*.c indexArchive/*.c

run:
	./programaTrab