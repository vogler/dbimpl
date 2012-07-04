/*
 * BufferManager.cpp
 *
 *  Created on: May 6, 2012
 *      Author: user
 */

#include "BufferManager.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

BufferManager::~BufferManager() {
	pthread_rwlock_destroy(&buffer_latch);
	pthread_rwlock_destroy(&lruBuffer_latch);
	close(file);
}

BufferManager::BufferManager(const string& filename, unsigned size) {
	this->size = size;
	pthread_rwlock_init(&buffer_latch, NULL);
	pthread_rwlock_init(&lruBuffer_latch, NULL);

    if ((file = open(filename.c_str(), O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
		cerr << "cannot open file '" << filename << "': " << strerror(errno) << endl;
		exit(1);
	}
}

BufferFrame& BufferManager::fixPage(unsigned pageId, bool exclusive){
	// read lock on hash map
	pthread_rwlock_rdlock(&buffer_latch);

	// if frame is not in cache try to cache it
	if(buffer.find(pageId) == buffer.end()){
		//cout << "not found..trying to cache" << endl;
		pthread_rwlock_unlock(&buffer_latch);
		cachePageFromFile(pageId);
		pthread_rwlock_rdlock(&buffer_latch);
	}

	// try to get page from cache
	if(buffer.find(pageId) != buffer.end()){
		// acquire appropriate lock
		assert(buffer[pageId] != NULL);
		if(exclusive==true){
			pthread_rwlock_wrlock(&(buffer[pageId]->latch));
			// cout << pageId << " wr_locked\n";
		} else {
			pthread_rwlock_rdlock(&(buffer[pageId]->latch));
			// cout << pageId << " rd_locked\n";
		}
		pthread_rwlock_unlock(&buffer_latch);
		assert(buffer[pageId] != NULL);
		return *buffer[pageId];
	}

	// release lock on hash map
	pthread_rwlock_unlock(&buffer_latch);
	// throw an error
	cerr << "Cache Error: Page could not be cached!" << endl;
	throw;
}


// this method was just used for testing
void BufferManager::printBufferStatus(){
	cout << "--- printing buffer status..." << endl;
	int sz = buffer.size();
	cout << "	hash map buffer size = " << sz;
	cout << ", contains pages: ";
	for(vector<BufferFrame*>::iterator i = lruBuffer.begin(); i != lruBuffer.end(); ++i) {
		if(buffer.find((*i)->pageId) != buffer.end()) {
			int lockTest = pthread_rwlock_trywrlock(&((*i)->latch));
			cout << (*i)->pageId << "(" << lockTest << "), ";
			if(lockTest == 0) pthread_rwlock_unlock(&((*i)->latch));
		}
	}
	cout << endl;

	sz = lruBuffer.size();
	cout << "	lruBuffer size = " << sz;
	cout << ", contains pages: ";
	for(int i = 0; i < sz; ++i){
		int lockTest = pthread_rwlock_trywrlock(&(lruBuffer[i]->latch));
		cout << lruBuffer[i]->pageId << "(" << lockTest << "), ";
		if(lockTest == 0) pthread_rwlock_unlock(&(lruBuffer[i]->latch));
	}
	cout << endl;
	cout << "	finished printing buffer status..." << endl;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	// Adjust the lru-list.
	pthread_rwlock_wrlock(&lruBuffer_latch);
	bool foundFrame = false;

	//cout << "trying to unfix page with id " << frame.pageId << endl;
	//printBufferStatus();

	BufferFrame *frameInBuffer = NULL;
	for(vector<BufferFrame*>::iterator i = lruBuffer.begin(); i != lruBuffer.end(); ++i){
		if((*i)->pageId == frame.pageId){
			frameInBuffer = *i;
			lruBuffer.erase(i);
			lruBuffer.push_back(frameInBuffer);

			foundFrame = true;
			if(isDirty){
				frameInBuffer->writeDataToFile();
			}
			int er = pthread_rwlock_unlock(&(frameInBuffer->latch));
			// pthread_rwlock_unlock(&(frame.latch));
			if(er!=0)
				cerr << "result code of unlocking frame: " << er;
			// cout << frame.pageId << " unlocked\n";
			break;
		}
	}
	pthread_rwlock_unlock(&lruBuffer_latch);

	//Error handling.
	if(!foundFrame){
		cerr << "Can not unfix a frame which is not cached!" << endl;
	}
}

// gets the requested data from file and puts a new bufferFrame into the hashmap
// if required, this method replaces frames in the buffer
void BufferManager::cachePageFromFile(unsigned pageId){
	// lock hash table and lru queue
	pthread_rwlock_wrlock(&buffer_latch);
	pthread_rwlock_wrlock(&lruBuffer_latch);

	// if buffer is full, replace a frame in buffer with page from file
	if(lruBuffer.size() >= size){
		// Search for the first lockable buffer frame
		BufferFrame *toBeFreed = NULL;
		for(vector<BufferFrame*>::iterator i = lruBuffer.begin(); i != lruBuffer.end(); ++i){
			// try to lock this frame, if not possible continue with next frame in lruBuffer
			int lockResult = pthread_rwlock_trywrlock(&((*i)->latch));
			//cout << "lockResult: " << lockResult << " for pageId " << (*i)->pageId << endl;
			if(lockResult == 0) {
				pthread_rwlock_unlock(&((*i)->latch));
				// cout << (*i)->pageId << " wr_locked and unlocked\n";
				toBeFreed = *i;
				// if we found our candidate frame, delete it from the buffers and deallocate the memory
				int cnt = buffer.erase(toBeFreed->pageId);	// delete from hash map
				assert(cnt == 1);
				toBeFreed->writeDataToFile();		// save changes
				lruBuffer.erase(i);
				 //free mmapped memory
				if (munmap(toBeFreed->getData(), FRAME_SIZE) == -1) {
					cerr << "Error un-mmapping the file";
				}
				delete toBeFreed;					// deallocate memory

				break;
			}
		}

		if(toBeFreed==NULL){
			cerr<<"no candidate was found => release locks and return!!!";
			pthread_rwlock_unlock(&lruBuffer_latch);
			pthread_rwlock_unlock(&buffer_latch);
			return;
		}

	}
	// add new frame to hash map and append to end of lruBuffer
	BufferFrame *bf = new BufferFrame(file, pageId);
	buffer[pageId] = bf;
	lruBuffer.push_back(bf);

	// release locks
	pthread_rwlock_unlock(&lruBuffer_latch);
	pthread_rwlock_unlock(&buffer_latch);
}
