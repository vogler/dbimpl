/*
 * BufferManager.h
 *
 *  Created on: May 6, 2012
 *      Author: user
 */

#ifndef BUFFERMANAGER_H_
#define BUFFERMANAGER_H_

using namespace std;
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <pthread.h>
#include "BufferFrame.h"

class BufferManager {
private:
	map < unsigned, BufferFrame* > buffer;
	pthread_rwlock_t buffer_latch;

	vector < BufferFrame* > lruBuffer;
	pthread_rwlock_t lruBuffer_latch;

	unsigned size;
	int file;
	void cachePageFromFile(unsigned pageId);

	void printBufferStatus();

public:
	~BufferManager();
	BufferManager(const string& filename, unsigned size);
	BufferFrame& fixPage(unsigned pageId, bool exclusive);
	void unfixPage(BufferFrame& frame, bool isDirty);
};

#endif /* BUFFERMANAGER_H_ */
