CXXFLAGS = -g -O2 -Wall -fmessage-length=0 -std=c++0x

OBJS =		./buffer/BufferManager.o ./buffer/BufferFrame.o
#OBJS += 	./testing/buffertest.o
OBJS += 	./segments/SegmentManager.o ./segments/SPSegment.o ./segments/Segment.o
OBJS +=		./bplustrees/InnerNode.o ./bplustrees/LeafNode.o ./bplustrees/BPlusTreeSegment.o ./bplustrees/BPTree.o
#OBJS +=	./bplustrees/BPTreesRAMImplementation.o
OBJS +=		./parser/Parser.o ./parser/Schema.o
OBJS +=		./schema/SchemaSegment.o
OBJS +=		./operators/Register.o ./operators/Print.o ./operators/TableScan.o ./operators/Operator.o
OBJS +=		./PlanReader/Plan.o ./operators/MergeJoin.o ./operators/Project.o ./operators/ScanFile.o ./operators/Select.o ./operators/Sort.o 
OBJS +=		./lock/LockManager.o

#OBJS +=		./SimiopaDB.o
#OBJS +=		./testing/operatorTest.o
OBJS +=		./testing/lockTest.o


LIBS =	-lpthread

TARGET =	test

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(CXXFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
