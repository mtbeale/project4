all: p4

p4: p4.o
        g++ -Wall -g -o 1730sh p4.o

p4.o: p4.cpp
        g++ -Wall -std=c++14 -g -O0 -pedantic-errors -c p4.cpp

clean:
        rm -rf *.o
        rm -rf 1730sh
