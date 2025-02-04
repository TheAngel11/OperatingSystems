all: Arda IluvatarSon
semaphore_v2.o: semaphore_v2.c semaphore_v2.h
	gcc -c -Wall -Wextra -g semaphore_v2.c
commands.o: Iluvatar/commands.c Iluvatar/commands.h semaphore_v2.h
	gcc -c -Wall -Wextra -g -lrt Iluvatar/commands.c
sharedFunctions.o: sharedFunctions.c sharedFunctions.h
	gcc -c -Wall -Wextra -g sharedFunctions.c
gpc.o: gpc.c gpc.h
	gcc -c -Wall -Wextra -g gpc.c
icp.o: icp.c icp.h semaphore_v2.h
	gcc -c -Wall -Wextra -g icp.c
server.o: server.c server.h
	gcc -c -Wall -Wextra -g server.c
client.o: client.c client.h
	gcc -c -Wall -Wextra -g client.c
IluvatarSon.o: Iluvatar/IluvatarSon.c definitions.h semaphore_v2.h
	gcc -c -Wall -Wextra -g -lrt Iluvatar/IluvatarSon.c
bidirectionallist.o: bidirectionallist.c bidirectionallist.h
	gcc -c -Wall -Wextra -g bidirectionallist.c
Arda.o: ArdaServer/Arda.c definitions.h
	gcc -c -Wall -Wextra -g ArdaServer/Arda.c
IluvatarSon: IluvatarSon.o semaphore_v2.o commands.o sharedFunctions.o bidirectionallist.o gpc.o icp.o client.o server.o
	gcc IluvatarSon.o semaphore_v2.o commands.o sharedFunctions.o bidirectionallist.o gpc.o icp.o client.o server.o -o IluvatarSon -Wall -Wextra -lpthread -g  -lrt
Arda: Arda.o sharedFunctions.o bidirectionallist.o gpc.o server.o
	gcc Arda.o sharedFunctions.o bidirectionallist.o gpc.o server.o -o Arda -Wall -Wextra -lpthread -g
clean:
	rm -f *.o
	rm -f IluvatarSon
	rm -f Arda
