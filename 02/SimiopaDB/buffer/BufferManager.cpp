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
		pthread_rwlock_unlock(&buffer_latch);
		cachePageFromFile(pageId);
		pthread_rwlock_rdlock(&buffer_latch);
	}

	// try to get page from cache
	if(buffer.find(pageId) != buffer.end()){
		// acquire appropriate lock
		if(exclusive==true){
			pthread_rwlock_wrlock(&(buffer[pageId]->latch));
		} else {
			pthread_rwlock_rdlock(&(buffer[pageId]->latch));
		}
		pthread_rwlock_unlock(&buffer_latch);
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
	cout << "printing buffer status..." << endl;
	int sz = buffer.size();
	cout << "hash map buffer size = " << sz;
	cout << ", contains pages: ";
	for(int i = 0; i < sz; ++i){
		cout << buffer[i]->pageId << ", ";
	}
	cout << endl;

	sz = lruBuffer.size();
	cout << "lruBuffer size = " << sz;
	cout << ", contains pages: ";
	for(int i = 0; i < sz; ++i){
		cout << lruBuffer[i]->pageId << ", ";
	}
	cout << endl;
	cout << "finished printing buffer status..." << endl;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	// Adjust the lru-list.
	pthread_rwlock_wrlock(&lruBuffer_latch);
	bool foundFrame = false;
	int sz = lruBuffer.size();

	//cout << "trying to unfix page with id " << frame.pageId << endl;
	//printBufferStatus();

	for(int i = 0; i < sz; i++){
		if(lruBuffer[i]->pageId == frame.pageId){

			//cout << "found page in lru buffer...unfixing" << endl;
			//printBufferStatus();
			lruBuffer.push_back(lruBuffer[i]);
			lruBuffer.erase(lruBuffer.begin()+i);
			//cout << "erased..." << endl;
			//printBufferStatus();
			//cout << "pushed..." << endl;
			//printBufferStatus();

			foundFrame = true;
			if(isDirty){
				frame.writeDataToFile();
			}
			pthread_rwlock_unlock(&(*lruBuffer.end())->latch);

			//cout << "unlocked latch of page " << lruBuffer[i]->pageId << ", finished unfixing----------------" << endl;

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
	if(buffer.size() >= size){
		// Search for the first lockable buffer frame
		BufferFrame *toBeFreed = NULL;
		for(vector<BufferFrame*>::iterator i = lruBuffer.begin(); i != lruBuffer.end(); ++i){
			// try to lock this frame, if not possible continue with next frame in lruBuffer
			if(pthread_rwlock_trywrlock(&((*i)->latch)) == 0){
				// lock acquired, saving candidate to be replaced
				toBeFreed = *i;
				lruBuffer.erase(i);
				break;
			}
		}

		// no candidate was found => release locks and return
		if(toBeFreed==NULL){
			pthread_rwlock_unlock(&lruBuffer_latch);
			pthread_rwlock_unlock(&buffer_latch);
			return;
		}

		// if we found and locked our candidate frame, delete it from the buffers and deallocate the memory
		buffer.erase(toBeFreed->pageId);	// delete from hash map
		toBeFreed->writeDataToFile();		// save changes
		delete toBeFreed;					// deallocate memory
	}
	// add new frame to hash map and append to end of lruBuffer
	BufferFrame *bf = new BufferFrame(file, pageId);
	buffer[pageId] = bf;
	lruBuffer.push_back(bf);

	// release locks
	pthread_rwlock_unlock(&lruBuffer_latch);
	pthread_rwlock_unlock(&buffer_latch);
}
