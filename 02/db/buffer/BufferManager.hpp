#include "BufferFrame.hpp"
#include <string>
#include <deque>
#include <map>

class BufferManager
{
  public:
    BufferManager(const std::string& filename, unsigned size);
    ~BufferManager();

    BufferFrame& fixPage(unsigned pageId, bool exclusive);
    void unfixPage(BufferFrame& frame, bool isDirty);
 
  private:
    const std::string& filename;
    unsigned size;
    std::map<unsigned, BufferFrame> frames;
    std::deque<BufferFrame> lru;
    int file;
    int filesize;
    int* buffer;
};
