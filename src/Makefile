main:TTTD.o
	g++ -o main main.cpp TTTD.cpp bpgraph.cpp bpgraph.h TTTD.h glo.h debug.h subProblem.h -lssl -lboost_filesystem-gcc42-mt -lpython2.5 -ltokyocabinet -I/usr/include/python2.5
debug:
	g++ -pg -g -o main main.cpp TTTD.cpp bpgraph.cpp bpgraph.h TTTD.h glo.h debug.h subProblem.h -lssl -lboost_filesystem-gcc42-mt -lpython2.5 -ltokyocabinet -I/usr/include/python2.5
clear:
	rm main *.o  
