all: main

CXX?=g++
CXXFLAGS=-std=c++0x -Wall -O0 -g

Plan.o: Plan.cpp Plan.hpp Makefile
	$(CXX) $(CXXFLAGS) -c Plan.cpp -o Plan.o

main: Plan.o main.cpp Makefile
	$(CXX) $(CXXFLAGS) Plan.o main.cpp -o main