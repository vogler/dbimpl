test_buf: testing/BufferManagerTest.cpp Makefile
	g++ -pthread -std=c++0x -I./buffer -g -O2 -Wall testing/BufferManagerTest.cpp buffer/BufferFrame.cpp buffer/BufferManager.cpp buffer/SPSegment.cpp buffer/SegmentManager.cpp  buffer/SlottedPage.cpp  buffer/SISegment.cpp  buffer/Segment.cpp -o test_buf

test_slot: testing/slottedTest.cpp buffer/Record.hpp Makefile
	g++ -pthread -std=c++0x -I./ -g -O2 -Wall testing/slottedTest.cpp buffer/Record.hpp buffer/BufferFrame.cpp buffer/BufferManager.cpp buffer/SPSegment.cpp buffer/SegmentManager.cpp  buffer/SlottedPage.cpp  buffer/SISegment.cpp  buffer/Segment.cpp -o test_slot

test_tree: testing/BTreeTest.cpp btree/BTree.hpp Makefile
	g++ -pthread -std=c++0x -I./ -g -O2 -Wall btree/BTree.hpp testing/BTreeTest.cpp -o test_tree
	
clean:
	rm -rf test_*

all:	test_tree