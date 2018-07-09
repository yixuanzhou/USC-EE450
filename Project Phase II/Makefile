PROG = paxos
CC = g++
CPPFLAGS = -std=c++11 -pthread -lsocket -lnsl -lresolv
LDFLAGS = -pthread
OBJS = main.o Acceptor.o Proposer.o Learner.o
$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)
main.o :
	$(CC) $(CPPFLAGS) -c main.cpp
Acceptor.o : Acceptor.h
	$(CC) $(CPPFLAGS) -c Acceptor.cpp
Proposer.o : Proposer.h
	$(CC) $(CPPFLAGS) -c Proposer.cpp
Learner.o : Learner.h
	$(CC) $(CPPFLAGS) -c Learner.cpp
clean :
	rm -f core $(PROG) $(OBJS)
