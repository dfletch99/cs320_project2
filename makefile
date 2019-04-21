CFLAGS = -Wall -Wextra -DDEBUG -g -std=c++14

all: main.o Cache.h
	g++ -o cache main.o
	
main.o:
	g++ -c $(CFLAGS) main.cpp -o main.o

test: all
	./cache trace1.txt out1.txt
	./cache trace2.txt out2.txt
	./cache trace3.txt out3.txt

clean:
	rm -rf *.o cache out1.txt out2.txt out3.txt
