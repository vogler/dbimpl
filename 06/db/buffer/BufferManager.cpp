/*
 * BufferManager.cpp
 *
 *  Created on: May 7, 2012
 *      Author: tinyos
 */

#include "BufferManager.hpp"
#include  <cstring>


BufferManager::BufferManager(const string& filename, unsigned size):
_filename(filename), _size(size)
{
	fstream(filename.c_str(), fstream::out).close();
	_fstream = new fstream(filename.c_str(), fstream::in | fstream::out);
	if (_fstream->fail()) {
		cout<<strerror( errno );
		throw 1;
	}

	pthread_mutexattr_t a;
	pthread_mutexattr_init(&a);
	pthread_mutexattr_settype(&a,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&this->_fstreamMutex, &a);
	pthread_mutex_init(&this->_bufferMutex, &a);
	pthread_mutex_init(&this->_lifoMutex, &a);
	pthread_mutexattr_destroy(&a);
}

BufferManager::~BufferManager()
{
	pthread_mutex_lock(&this->_bufferMutex);

	while (this->_buffer.begin() != this->_buffer.end()) {
		this->_deleteFrame(this->_buffer.begin()->second.second);
	}

	_fstream->close();

	pthread_mutex_unlock(&this->_bufferMutex);
}

BufferFrame& BufferManager::fixPage(unsigned pageId, bool exclusive)
{
	pthread_mutex_lock(&this->_bufferMutex); // lock buffer so no other thread can interfere while we get/add a frame
	pthread_mutex_lock(&this->_lifoMutex);

	//cout<<"fixpage"<<pageId<< " "<< (exclusive?"exclusive":"NON-exclusive")<<" (buffer="<<this->_buffer.size()<<")"<<endl;
BufferFrame* frame;
	if (this->_buffer.find(pageId) != this->_buffer.end()) {//cout<<1<<endl;
		frame = this->_buffer.find(pageId)->second.second;
		pthread_mutex_lock(&frame->mutex);
		frame->state |= STATES::fixed;
		pthread_mutex_unlock(&this->_lifoMutex); // release buffer
		pthread_mutex_unlock(&this->_bufferMutex);
	} else {
		/* ensure a place for our new frame is available */
		if (this->_buffer.size() >= this->_size) { // if buffer is full, try to clean it up
			cout<<"cleaning up for page"<<pageId<<endl;
			// remove oldest unfixed entry
			for (list<BufferFrame*>::iterator it = this->_lifo.begin();
					it != this->_lifo.end();
					it++) {

				BufferFrame* frame = *it;
				if (!(frame->state & STATES::fixed)) {
					if (pthread_mutex_trylock(&frame->mutex) == 0) {
						pthread_mutex_unlock(&frame->mutex);
						this->_deleteFrame(frame);
						frame=NULL;
						break;
					}
				}
			}

			cout<<"     >"<<this->_buffer.size()<<endl;
			for (map<unsigned,pair<pthread_rwlock_t*,BufferFrame*>>::iterator it = this->_buffer.begin();
					it != this->_buffer.end();
					it++) {

				BufferFrame* frame = it->second.second;
				cout<<"     >"<<frame->getPageId()<<" = "<<(frame->state&STATES::fixed ? "fixed" : "unfixed");
			}
			cout<<endl<<endl;

			if (this->_buffer.size() >= this->_size) { // abort if buffer is still full
				pthread_mutex_unlock(&this->_lifoMutex); // release buffer
				pthread_mutex_unlock(&this->_bufferMutex);
				throw PageFixException;
			}
		}

		/*
		 * create empty frame object, lock it, push it to the buffer and immediately release the buffer
		 * => only access to the frame is blocked and other operations on the buffer can still be done
		 */
		frame = new BufferFrame(pageId, PAGE_SIZE, NULL);
		frame->state |= STATES::fixed; // mark as fixed

		pthread_mutex_lock(&frame->mutex); // lock frame, put frame into buffer
		pthread_rwlock_t* rwlock = new pthread_rwlock_t();
		pthread_rwlock_init(rwlock, NULL);
		this->_buffer.insert(pair<unsigned,pair<pthread_rwlock_t*,BufferFrame*>>(pageId,pair<pthread_rwlock_t*,BufferFrame*>(rwlock,frame)));
		pthread_mutex_unlock(&this->_lifoMutex); // release buffer
		pthread_mutex_unlock(&this->_bufferMutex);

		frame->setData(this->_readFrame(pageId)); // read frame
	}
	pthread_mutex_unlock(&frame->mutex);

	if (!exclusive) {
		pthread_rwlock_rdlock(this->_buffer.find(pageId)->second.first);
	} else {
		pthread_rwlock_wrlock(this->_buffer.find(pageId)->second.first);
	}

	return *frame;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
	pthread_rwlock_unlock(this->_buffer.find(frame.getPageId())->second.first);
	pthread_mutex_lock(&frame.mutex);

	if (isDirty) {
		frame.state |= STATES::dirty;
	}

	// mark thread as unfixed if it isn't used anymore
	if (pthread_rwlock_trywrlock(this->_buffer.find(frame.getPageId())->second.first) == 0) {
//cout<<"UNFIX page"<<frame.getPageId()<<endl;
		frame.state &= ~STATES::fixed;

		pthread_mutex_lock(&this->_bufferMutex); // lock buffer so no other thread can interfere while we get/add a frame
		pthread_mutex_lock(&this->_lifoMutex);

		// remove from lifo
		for (list<BufferFrame*>::iterator it = this->_lifo.begin();
				it != this->_lifo.end();
				it++) {

			if (frame.getPageId() == (*it)->getPageId()) {
				this->_lifo.remove(*it);
				break;
			}
		}

		// add to the end of the lifo
		this->_lifo.push_back(&frame);

		pthread_mutex_unlock(&this->_lifoMutex);
		pthread_mutex_unlock(&this->_bufferMutex);

		pthread_rwlock_unlock(this->_buffer.find(frame.getPageId())->second.first);
	}
	pthread_mutex_unlock(&frame.mutex);
}

void BufferManager::_deleteFrame(BufferFrame* frame)
{
	if (frame==NULL) return;
//cout<<"deleting page"<<frame->getPageId()<<endl;
	pthread_mutex_lock(&frame->mutex);

	this->_writeFrame(frame); // write frame (_writeFrame does only write frame if frame is dirty..)

	pthread_mutex_lock(&this->_bufferMutex); // remove from buffer
	pthread_mutex_lock(&this->_lifoMutex);
	delete this->_buffer.find(frame->getPageId())->second.first;
	this->_buffer.erase(this->_buffer.find(frame->getPageId()));
	for (list<BufferFrame*>::iterator it = this->_lifo.begin();
			it != this->_lifo.end();
			it++) {
		if ((*it) == frame) {
			this->_lifo.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&this->_lifoMutex);
	pthread_mutex_unlock(&this->_bufferMutex);

	free(frame->getData()); // free occupied resource

	pthread_mutex_unlock(&frame->mutex); // free mutex

	//delete frame; // invalidate pointer
}

void* BufferManager::_readFrame(unsigned pageId)
{
	pthread_mutex_lock(&this->_fstreamMutex);

	// seek to page position
	unsigned pageAddress = this->_getPageAddress(pageId);
	this->_fstream->seekg(pageAddress, ios::beg); // will seek to specific position even if the fail isn't big enough

	// read page
	void* frame = malloc(BufferManager::PAGE_SIZE);
	this->_fstream->read(reinterpret_cast<char*>(frame), BufferManager::PAGE_SIZE);
	if (!(*this->_fstream)) {
		for (unsigned i=this->_fstream->gcount(); i<BufferManager::PAGE_SIZE; i++) {
			reinterpret_cast<char*>(frame)[i] = 0;
		}
		this->_fstream->clear();
	}

	pthread_mutex_unlock(&this->_fstreamMutex);

	return frame;
}

void BufferManager::_writeFrame(BufferFrame* frame)
{
	// abort if either NULL pointer is given or the page is not dirty and unfixed.
	if (frame==NULL || !(frame->state&STATES::dirty) || (frame->state&STATES::fixed) ) return;

	pthread_mutex_lock(&frame->mutex); // lock frame

	// write frame
	unsigned pageAddress = this->_getPageAddress(frame->getPageId());

	pthread_mutex_lock(&this->_fstreamMutex); // lock file stream
	this->_fstream->seekp(pageAddress, ios::beg);
	this->_fstream->write(reinterpret_cast<char*>(frame->getData()), BufferManager::PAGE_SIZE);
	if (!(*this->_fstream)) {
		this->_fstream->clear();
	}
	cout<<"WRITE FRAME ("<<strerror( errno )<<"): "<<pageAddress<<endl;
	pthread_mutex_unlock(&this->_fstreamMutex); // release file stream

	pthread_mutex_unlock(&frame->mutex); // release frame
}

unsigned BufferManager::_getPageAddress(unsigned pageId)
{
	return pageId * BufferManager::PAGE_SIZE;
}

