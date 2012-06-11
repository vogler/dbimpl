#include "BufferManager.hpp"
#include <iostream>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <cstring>


using namespace std;

const unsigned pagesize = 4096/sizeof(int);


BufferManager::BufferManager(const std::string& filename, unsigned size)
: filename(filename), size(size)
{
  cout << "using file " << filename << endl;
  if ((this->file = open(filename.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
  cerr << "cannot open file '" << filename << "': " << strerror(errno) << endl;
    exit(1);
  }
  // read file into buffer
  this->buffer = (int*) malloc(size*pagesize);
  this->filesize = read(this->file, this->buffer, sizeof(this->buffer));

  int* ptr = this->buffer;
  for(int i=0; i<size; i++){
    ptr += pagesize;
    BufferFrame bf(i, ptr);
    this->frames[i] = bf;
    this->lru.push_front(bf);
  }
}

BufferManager::~BufferManager(){
  free(this->buffer);
  close(this->file);
}

BufferFrame& BufferManager::fixPage(unsigned pageId, bool exclusive){
  if(this->frames.count(pageId) < 1 && this->frames.size() == this->frames.max_size()){ // frame not in map and buffer full -> make room
    BufferFrame& bf2swap = this->lru.front();  // take head of queue
    this->unfixPage(bf2swap, bf2swap.isDirty); // unfix it, TODO can I do this? search for first unfixed one?
    this->frames.erase(bf2swap.pageId);        // remove from map
    this->lru.pop_front();                     // remove from queue
  }
  BufferFrame& bf = this->frames[pageId];      // gets reference, if there is no element for the key it creates one
  if(exclusive){
    pthread_rwlock_wrlock(&bf.lock);
  }else{
    pthread_rwlock_rdlock(&bf.lock);
  }
  
  pthread_rwlock_unlock(&bf.lock);;
  return bf;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty){
  if(frame.isDirty){ // write back to disk
    write(this->file, frame.data, pagesize);
    frame.isDirty = false;
  }
  frame.fixed = false;
  this->lru.erase(*frame);
  this->lru.push_back(frame);
}


BufferManager* bm;

int main(int argc, char** argv) {
  bm = new BufferManager("blubb", 5);
  return 0;
}
