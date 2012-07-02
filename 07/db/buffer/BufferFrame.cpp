/*
 * BufferFrame.cpp
 *
 *  Created on: May 7, 2012
 *      Author: tinyos
 */

#include "BufferFrame.hpp"

BufferFrame::BufferFrame(unsigned pageId, size_t size, void* data):
state(0), _size(size), _data(data), _pageId(pageId)
{
	pthread_rwlock_init(&this->rwlock, NULL);

	pthread_mutexattr_t a;
	pthread_mutexattr_init(&a);
	pthread_mutexattr_settype(&a,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&this->mutex, &a);
	pthread_mutexattr_destroy(&a);
}

BufferFrame::~BufferFrame() {
}
