CXX = g++
CXXFLAGS = -O -std=c++20 -Wall -I ../wildcardtl/include

LDFLAGS = -O

LIBS = 

all: extensions

extensions: main.o
	$(CXX) $(LDFLAGS) -o extensions main.o $(LIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -o main.o -c main.cpp

library:

binary:

clean:
	- rm *.o
	- rm extensions
