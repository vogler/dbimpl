CXXFLAGS = -g -O2 -Wall -fmessage-length=0

OBJS =		./testing/buffertest.o ./buffer/BufferManager.o ./buffer/BufferFrame.o

LIBS =	-lpthread

TARGET =	SimiopaDB

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(CXXFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
