testStreamParser:streamParserDemo.o
	g++ -o testStreamParser streamParserDemo.o -lpthread -lstdc++ -ldl -lrt ./libStreamParser.a
main.o:streamParserDemo.cpp
	g++ -o streamParserDemo.o -c streamParserDemo.cpp 

clean:
	rm -rf *.o \
	testStreamParser

