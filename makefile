all: IluvatarSon
commands.o: commands.c commands.h
	gcc -c -Wall -Wextra commands.c
IluvatarSon.o: IluvatarSon.c definitions.h
	gcc -c -Wall -Wextra IluvatarSon.c
IluvatarSon: IluvatarSon.o commands.o
	gcc IluvatarSon.o commands.o -o IluvatarSon
clean:
	rm *.o
	rm IluvatarSon
