INCLUDES = -I./utils -I./header -I./registry -I./functions -I./indexArchive -I./Graph

all:
	gcc $(INCLUDES) -o programaTrab main.c fornecidas.c utils/*.c header/*.c registry/*.c functions/*.c indexArchive/*.c Graph/*.c

run:
	./programaTrab