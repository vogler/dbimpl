#include "BufferManager.hpp"
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

using namespace std;

BufferManager* bm;

int main(int argc, char** argv) {
  bm = new BufferManager("blubb", 5);
  return 0;
}
