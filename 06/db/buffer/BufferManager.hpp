/*
 * BufferManager.hpp
 *
 *  Created on: May 7, 2012
 *      Author: tinyos
 */

#ifndef BUFFERMANAGER_HPP_
#define BUFFERMANAGER_HPP_


#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <memory>
#include <map>
#include <list>
#include <exception>
#include <pthread.h>
#include "BufferFrame.hpp"
using namespace std;




class BufferManager
{
public:
	/***
	 * public member
	 */
	/** size of a page in byte */
	const static unsigned PAGE_SIZE=16*1024;

	static class PageFixException: public exception
	{
	} PageFixException;


	/***
	 * public helper
	 */
	BufferManager(const string& filename, unsigned size);
	~BufferManager();
	BufferFrame& fixPage(unsigned pageId, bool exclusive);
	void unfixPage(BufferFrame& frame, bool isDirty);

protected:
	/** protected helper **/
	void* _readFrame(unsigned pageId);
	void _writeFrame(BufferFrame*);
	void _deleteFrame(BufferFrame*);
	unsigned _getPageAddress(unsigned pageId);

	/** protected member **/
	// buffer
	pthread_mutex_t _bufferMutex;
	map<unsigned,pair<pthread_rwlock_t*,BufferFrame*>> _buffer;
	pthread_mutex_t _lifoMutex;
	list<BufferFrame*> _lifo;

	// iostream
	pthread_mutex_t _fstreamMutex;
	fstream _fstream;

	// constructor arguments
	string _filename;
	unsigned _size;

};



#endif /* BUFFERMANAGER_HPP_ */
