/*
 * BufferFrame.cpp
 *
 *  Created on: May 6, 2012
 *      Author: user
 */

#include "BufferFrame.h"
#include <iostream>

BufferFrame::BufferFrame(int fd, unsigned pageId) {
    data = (char *)mmap(0, FRAME_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, pageId*FRAME_SIZE);
	if (data == MAP_FAILED) {
		cerr << "mmap failed";
		throw;
	}
	pthread_rwlock_init(&latch, NULL);

	this->pageId = pageId;
}

BufferFrame::BufferFrame(){}

BufferFrame::~BufferFrame() {
	pthread_rwlock_destroy(&latch);

// free mmapped memory
//	if (munmap(data, FRAME_SIZE) == -1) {
//		cerr << "Error un-mmapping the file";
//		// Decide here whether to close(fd) and exit() or not. Depends...
//	}

}

void* BufferFrame::getData(){
	// TODO
	return reinterpret_cast<void*>(data);
}

void BufferFrame::writeDataToFile(){
	// write possibly modified data back to file
	msync(data, FRAME_SIZE, MS_SYNC);
}
