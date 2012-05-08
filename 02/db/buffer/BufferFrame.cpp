#include "BufferFrame.hpp"
#include <pthread.h> 

using namespace std;

BufferFrame::BufferFrame()
:isDirty(false)
{
  
}

BufferFrame::BufferFrame(unsigned pageId, void* data)
: pageId(pageId), isDirty(false), data(data)
{
//  pthread_rwlockattr_t attr; 
//  pthread_rwlockattr_init(&attr); 
//  pthread_rwlockattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
  pthread_rwlock_init(&this->lock, NULL);
}

BufferFrame::~BufferFrame(){
  
}

void* BufferFrame::getData(){
  return data;
}
