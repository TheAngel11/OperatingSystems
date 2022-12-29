all: Arda IluvatarSon
commands.o: Iluvatar/commands.c Iluvatar/commands.h
	gcc -c -Wall -Wextra -g -lrt Iluvatar/commands.c
sharedFunctions.o: sharedFunctions.c sharedFunctions.h
	gcc -c -Wall -Wextra -g sharedFunctions.c
gpc.o: gpc.c gpc.h
	gcc -c -Wall -Wextra -g gpc.c 
server.o: server.c server.h
	gcc -c -Wall -Wextra -g server.c
client.o: client.c client.h
	gcc -c -Wall -Wextra -g client.c
IluvatarSon.o: Iluvatar/IluvatarSon.c definitions.h
	gcc -c -Wall -Wextra -g -lrt Iluvatar/IluvatarSon.c
bidirectionallist.o: bidirectionallist.c bidirectionallist.h
	gcc -c -Wall -Wextra -g bidirectionallist.c
Arda.o: ArdaServer/Arda.c definitions.h
	gcc -c -Wall -Wextra -g ArdaServer/Arda.c
IluvatarSon: IluvatarSon.o commands.o sharedFunctions.o bidirectionallist.o gpc.o client.o server.o
	gcc IluvatarSon.o commands.o sharedFunctions.o bidirectionallist.o gpc.o client.o server.o -o IluvatarSon -Wall -Wextra -lpthread -g  -lrt
Arda: Arda.o sharedFunctions.o bidirectionallist.o gpc.o server.o
	gcc Arda.o sharedFunctions.o bidirectionallist.o gpc.o server.o -o Arda -Wall -Wextra -lpthread -g
clean:
	rm -f *.o
	rm -f IluvatarSon
	rm -f Arda
