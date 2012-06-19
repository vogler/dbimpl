#ifndef H_Record_HPP
#define H_Record_HPP

#include <string.h>
#include <cstdlib>
using namespace std;

// A simple Record implementation (without custom allocator and error handling)
class Record {
   unsigned len;
   char* data;

public:
   Record& operator=(Record& rhs) = delete;
   Record(Record& t) = delete;
   Record(Record&& t) : len(t.len), data(t.data) {
      t.data = 0;
      t.len = 0;
   }
   explicit Record(unsigned len, const char* const ptr) : len(len) {
      data = static_cast<char*>(malloc(len));
      memcpy(data, ptr, len);
   }
   const char* getData() const {
      return data;
   }
   unsigned getLen() const {
      return len;
   }
   ~Record() {
      free(data);
   }
};

#endif
