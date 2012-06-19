/*
 * BufferFrame.hpp
 *
 *  Created on: May 7, 2012
 *      Author: tinyos
 */

#ifndef BUFFERFRAME_HPP_
#define BUFFERFRAME_HPP_

#include <pthread.h>
#include <iostream>
using namespace std;

enum STATES {
		dirty= 1,
		fixed= 2,
	};

class BufferFrame
{
public:
	/** public member **/
	unsigned state;
	pthread_mutex_t mutex; // mutex for write operations on this frame
	pthread_rwlock_t rwlock; // read write lock for access control


	/** public methods **/
	BufferFrame(unsigned pageId, size_t size, void* data);
	~BufferFrame();
	void setData(void* data) {
		this->_data = data;
	}
	void* getData() { // inline since getter
		return this->_data;
	}
	unsigned getPageId() { // inline since getter
		return this->_pageId;
	}
	size_t getSize() {
		return _size;
	}

protected:
	size_t _size;
	void* _data;
	unsigned _pageId;
};


#endif /* BUFFERFRAME_HPP_ */
