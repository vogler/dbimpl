/*
 * SISegment.cpp
 *
 *  Created on: May 29, 2012
 *      Author: user
 */

#include "SISegment.hpp"
#include <stdio.h>
#include <iostream>
#include <exception>
#include <typeinfo>
using namespace std;

SISegment::SISegment(BufferManager& bm, BufferFrame* page): Segment(bm, vector<BufferFrame*>(1,page)), _pageCount(0) {

	//
	// type | page Count | page1 | page2..
	// 0
	//
	char* ptr = reinterpret_cast<char*>(page->getData());
	unsigned curPageCount;
	do {
		SegmentType type = *reinterpret_cast<SegmentType*>(ptr);
		curPageCount=*reinterpret_cast<unsigned*>(ptr+sizeof(SegmentType));

		vector<unsigned> curPages;
		for (unsigned i=0; i<curPageCount; i++) {
			curPages.push_back(*reinterpret_cast<unsigned*>(ptr+sizeof(SegmentType)+sizeof(unsigned)*(i+1)));
			_pageCount++;
		}

		this->_segments.push_back(SegmentEntry(type,curPageCount,curPages));
	} while(curPageCount > 0);

}

SISegment::SegmentEntry SISegment::segmentEntry(SegmentID id) {
	if (id >= _segments.size()) {
		throw SegmentManager::IllegalSegmentException;
	}

	return _segments[id];
}

SegmentID SISegment::createSegment(Segment::SegmentType type, unsigned size) {

	vector<unsigned> pages;

	// insert into db
	char* ptr = reinterpret_cast<char*>(this->getPage(0).getData());
	*reinterpret_cast<SegmentType*>(ptr) = type;
	*reinterpret_cast<unsigned*>(ptr+sizeof(SegmentType)) = size;
	for (unsigned i=0; i<size; i++) {
		*reinterpret_cast<unsigned*>(ptr+sizeof(SegmentType)+sizeof(unsigned)*(i+1)) = _pageCount;
		pages.push_back(_pageCount);
		_pageCount++;
	}
	*reinterpret_cast<unsigned*>(ptr+sizeof(SegmentType)+sizeof(unsigned)*(size+1)) = 0;

	// insert into cache
	this->_segments.push_back(SegmentEntry(type, size, pages));

	return this->_segments.size()-1;
}

unsigned SISegment::pageCount() {
	return _pageCount;
}


