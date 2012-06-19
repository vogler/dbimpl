#include "SPSegment.hpp"

using namespace std;


SPSegment::SPSegment(BufferManager& bm, vector<BufferFrame*> pages) : Segment(bm, pages) {
	for (vector<BufferFrame*>::iterator it=pages.begin();
			it != pages.end();
			it++) {
		_slottedPages.insert( std::map<unsigned,SlottedPage>::value_type((*it)->getPageId(), SlottedPage(**it)));
	}
}

TID SPSegment::insert(const Record& r){
	TID* tid=NULL;

	for (map<unsigned,SlottedPage>::iterator it=_slottedPages.begin();
			it != _slottedPages.end();
			it++) {

		tid = it->second.getFreeSlot();
		if (tid != NULL) {
			it->second.occupySlot(*tid, r);
			break;
		}
	}
	if (tid == NULL) {
		throw SegmentFullException;
	}

	return *tid;
}

bool SPSegment::remove(TID tid){
	if (_slottedPages.find(SlottedPage::getPageID(tid)) != _slottedPages.end()) {
		return _slottedPages.find(SlottedPage::getPageID(tid))->second.freeSlot(tid);
	}

	return false;
}

const Record* SPSegment::lookup(TID tid){
	return _slottedPages.find(SlottedPage::getPageID(tid))->second.getRecord(tid);
}

bool SPSegment::update(TID tid, const Record& r){
	return _slottedPages.find(SlottedPage::getPageID(tid))->second.occupySlot(tid, r);
}
