/*
 * BufferFrame.h
 *
 *  Created on: May 6, 2012
 *      Author: user
 */

#ifndef BUFFERFRAME_H_
#define BUFFERFRAME_H_

using namespace std;
#include <pthread.h>


#define FRAME_SIZE 4096

class BufferFrame {
private:
	//unsigned char data[FRAME_SIZE];
	char *data;
public:
	unsigned pageId;
	BufferFrame(int fd, unsigned pageId);
	~BufferFrame();
	void* getData();
	pthread_rwlock_t latch;
	void writeDataToFile();
};

#endif /* BUFFERFRAME_H_ */
