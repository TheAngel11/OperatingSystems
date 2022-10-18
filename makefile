all: IluvatarSon
commands.o: commands.c commands.h
	gcc -c -Wall -Wextra commands.c
sharedFunctions.o: sharedFunctions.c sharedFunctions.h
	gcc -c -Wall -Wextra sharedFunctions.c
IluvatarSon.o: IluvatarSon.c definitions.h
	gcc -c -Wall -Wextra IluvatarSon.c
IluvatarSon: IluvatarSon.o commands.o sharedFunctions.o
	gcc IluvatarSon.o commands.o sharedFunctions.o -o IluvatarSon
clean:
	rm *.o
	rm IluvatarSon