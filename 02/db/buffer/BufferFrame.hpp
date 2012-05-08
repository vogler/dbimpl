#include <pthread.h> 

class BufferFrame
{
  public:
    BufferFrame();
    BufferFrame(unsigned pageId, void* data);
    ~BufferFrame();

    void* getData();
 
  public:
    unsigned pageId;
    bool isDirty;
    void* data;
    pthread_rwlock_t lock;
    bool fixed;
};
