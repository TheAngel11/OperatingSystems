all: IluvatarSon
commands.o: Iluvatar/commands.c Iluvatar/commands.h
	gcc -c -Wall -Wextra Iluvatar/commands.c
sharedFunctions.o: sharedFunctions.c sharedFunctions.h
	gcc -c -Wall -Wextra sharedFunctions.c
IluvatarSon.o: Iluvatar/IluvatarSon.c definitions.h
	gcc -c -Wall -Wextra Iluvatar/IluvatarSon.c
IluvatarSon: IluvatarSon.o commands.o sharedFunctions.o
	gcc IluvatarSon.o commands.o sharedFunctions.o -o IluvatarSon -Wall -Wextra
clean:
	rm *.o
	rm IluvatarSon
