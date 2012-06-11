#ifndef SPSEGMENT_HPP_
#define SPSEGMENT_HPP_

#include <iostream>
#include "BufferFrame.hpp"
#include "Record.hpp"
#include <vector>
#include <map>

using namespace std;

// typedef
struct TID {
  int pageID;
  int slotID;
  TID(const int pageID, const int slotID) : 
    pageID(pageID), slotID(slotID) {}
  bool operator<(const TID& A) const
    { return pageID<A.pageID && slotID<A.slotID; }
};

class SPSegment
{
public:
	/** public member **/
  const static unsigned PAGES=2;

	/** public methods **/
	SPSegment(vector<BufferFrame*> pages);
	~SPSegment();
	
	unsigned size();
	BufferFrame& getPage(unsigned id);
	
	TID insert(const Record& r);
	bool del(TID tid);
	Record* lookup(TID tid);
	bool update(TID tid, const Record& r);

protected:
	vector<BufferFrame*> pages;
	map<TID, const Record*> records;
	unsigned page;
	unsigned slot;
};


#endif /* SPSEGMENT_HPP_ */
