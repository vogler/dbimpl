/*
 * Segment.cpp
 *
 *  Created on: May 29, 2012
 *      Author: user
 */

#include <Segment.hpp>

Segment::Segment(BufferManager& bm, vector<BufferFrame*> pages): bm(bm), pages(pages) {
}

Segment::~Segment() {
	cout<<"DESTRUCT"<<endl;
	for (vector<BufferFrame*>::iterator it=this->pages.begin();
			it!=this->pages.end();
			it++) {
		this->bm.unfixPage((**it), true);
	}
}

unsigned Segment::size(){
	return this->pages.size();
}

BufferFrame& Segment::getPage(SegmentID id){
	return *this->pages[id];
}

