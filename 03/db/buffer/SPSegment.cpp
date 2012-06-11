#include "SPSegment.hpp"

using namespace std;


SPSegment::SPSegment(vector<BufferFrame*> pages) : pages(pages), page(0), slot(0) {
  
}

SPSegment::~SPSegment(){
  
}

unsigned SPSegment::size(){
  return this->pages.size();
}

BufferFrame& SPSegment::getPage(unsigned id){
  return *this->pages[id];
}

TID SPSegment::insert(const Record& r){
  //this->records.push_back(&r);
  TID tid(this->page, this->slot);
  this->records[tid] = &r;
  this->slot++;
  // r.getLen()
  return tid;
}

bool SPSegment::del(TID tid){
  //map<TID, const Record*>::iterator it;
  bool existed = this->records.count(tid) > 0;
  if(existed){
    //it = this->records.find(tid);
    //this->records.erase(it);
    delete this->records[tid];
  }
  return existed;
}

Record* SPSegment::lookup(TID tid){
  return (Record*)this->records[tid];
}

bool SPSegment::update(TID tid, const Record& r){
  bool existed = this->records.count(tid) > 0;
  this->records[tid] = &r;
  return existed;
}
