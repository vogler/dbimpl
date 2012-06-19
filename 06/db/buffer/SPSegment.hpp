#ifndef SPSEGMENT_HPP_
#define SPSEGMENT_HPP_

#include <iostream>
#include "BufferFrame.hpp"
#include "Record.hpp"
#include "Segment.hpp"
#include "SlottedPage.hpp"
#include <vector>
#include <map>

using namespace std;


/**
 * Slotted Page Segment
 */
class SPSegment: public Segment
{
	static class SegmentFullException: public exception
	{
	} SegmentFullException;


public:
	/** public member **/

	/** public methods **/
	SPSegment(BufferManager& bm, vector<BufferFrame*> pages);
	
	unsigned size();
	
	TID insert(const Record& r);
	bool remove(TID tid);
	const Record* lookup(TID tid);
	bool update(TID tid, const Record& r);
	unsigned slotsPerPage() {
		return _slottedPages.begin()->second.slotCount();
	}

protected:
	map<unsigned,SlottedPage> _slottedPages;
};


#endif /* SPSEGMENT_HPP_ */
