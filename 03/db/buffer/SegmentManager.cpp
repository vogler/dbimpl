#include "SegmentManager.hpp"

using namespace std;


SegmentManager::SegmentManager(BufferManager& bm) : bm(bm), si(bm.fixPage(0, true)) {
  //this->si = ;
  
}

SegmentManager::~SegmentManager(){
  bm.unfixPage(this->si, true);
}

unsigned SegmentManager::addSegment(){
  vector<BufferFrame*> pages;
  pages.push_back(&bm.fixPage((this->size()-1)*2+1, false));
  pages.push_back(&bm.fixPage((this->size()-1)*2+2, false));
  SPSegment* s = new SPSegment(pages);
  this->segments.push_back(s);
  return this->segments.size();
}

SPSegment& SegmentManager::getSegment(unsigned id){
  return *this->segments[id];
}

unsigned SegmentManager::size(){
  return this->segments.size();
}
