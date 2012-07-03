#include "SegmentManager.hpp"

using namespace std;


SegmentManager::SegmentManager(BufferManager& bm) : bm(bm) {
	BufferFrame* page = &bm.fixPage(0,true);
	this->si = new SISegment(bm, page);
}

SegmentManager::~SegmentManager(){
}

//SegmentID SegmentManager::createSegment(Segment::SegmentType type, unsigned size){
//	return this->si->createSegment(type, size);
//}
//
//Segment& SegmentManager::getSegment(SegmentID id){
//	SISegment::SegmentEntry entry = this->si->segmentEntry(id);
//
//	vector<BufferFrame*> pages;
//	for (unsigned i=0; i<entry.size; i++) {
//		pages.push_back(&bm.fixPage(entry.pages[i], false));
//	}
//
//	Segment* s=NULL;
//	switch (entry.type) {
//
//		case Segment::SegmentType::SP:
//		default:
//			s = new SPSegment(bm, pages);
//		break;
//	}
//	return *s;
//}

unsigned SegmentManager::size(){
  return this->si->pageCount();
}
