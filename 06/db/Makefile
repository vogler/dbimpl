test: testing/BufferManagerTest.cpp Makefile
	g++ -pthread -std=c++0x -I./buffer -g -O2 -Wall testing/BufferManagerTest.cpp buffer/BufferFrame.cpp buffer/BufferManager.cpp buffer/SPSegment.cpp buffer/SegmentManager.cpp -o test

clean:
	rm -rf test

all:	test