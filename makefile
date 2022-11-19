all: Arda IluvatarSon
commands.o: Iluvatar/commands.c Iluvatar/commands.h
	gcc -c -Wall -Wextra Iluvatar/commands.c
sharedFunctions.o: sharedFunctions.c sharedFunctions.h
	gcc -c -Wall -Wextra sharedFunctions.c
IluvatarSon.o: Iluvatar/IluvatarSon.c definitions.h
	gcc -c -Wall -Wextra Iluvatar/IluvatarSon.c
bidirectionallist.o: bidirectionallist.c bidirectionallist.h
	gcc -c -Wall -Wextra bidirectionallist.c
Arda.o: ArdaServer/Arda.c definitions.h
	gcc -c -Wall -Wextra ArdaServer/Arda.c
IluvatarSon: IluvatarSon.o commands.o sharedFunctions.o
	gcc IluvatarSon.o commands.o sharedFunctions.o bidirectionallist.o -o IluvatarSon -Wall -Wextra
Arda: Arda.o sharedFunctions.o bidirectionallist.o
	gcc Arda.o sharedFunctions.o bidirectionallist.o -o Arda -Wall -Wextra -lpthread
clean:
	rm -f *.o
	rm -f IluvatarSon
	rm -f Arda
