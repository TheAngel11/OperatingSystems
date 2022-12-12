all: Arda IluvatarSon
commands.o: Iluvatar/commands.c Iluvatar/commands.h
	gcc -c -Wall -Wextra Iluvatar/commands.c
sharedFunctions.o: sharedFunctions.c sharedFunctions.h
	gcc -c -Wall -Wextra sharedFunctions.c
gpc.o: gpc.c gpc.h
	gcc -c -Wall -Wextra gpc.c
server.o: server.c server.h
	gcc -c -Wall -Wextra server.c
client.o: client.c client.h
	gcc -c -Wall -Wextra client.c
IluvatarSon.o: Iluvatar/IluvatarSon.c definitions.h
	gcc -c -Wall -Wextra Iluvatar/IluvatarSon.c
bidirectionallist.o: bidirectionallist.c bidirectionallist.h
	gcc -c -Wall -Wextra bidirectionallist.c
Arda.o: ArdaServer/Arda.c definitions.h
	gcc -c -Wall -Wextra ArdaServer/Arda.c
IluvatarSon: IluvatarSon.o commands.o sharedFunctions.o bidirectionallist.o gpc.o client.o
	gcc IluvatarSon.o commands.o sharedFunctions.o bidirectionallist.o gpc.o client.o -o IluvatarSon -Wall -Wextra
Arda: Arda.o sharedFunctions.o bidirectionallist.o gpc.o server.o
	gcc Arda.o sharedFunctions.o bidirectionallist.o gpc.o server.o -o Arda -Wall -Wextra -lpthread
clean:
	rm -f *.o
	rm -f IluvatarSon
	rm -f Arda
