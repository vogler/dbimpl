/*
 * SPSegment.cpp
 *
 *  Created on: May 17, 2012
 *      Author: user
 */

#include "SPSegment.h"

namespace std {

SPSegment::SPSegment(SIEntry siEntry, BufferManager& bm) : Segment(siEntry, bm) {

}

SPSegment::~SPSegment() {
	// TODO Auto-generated destructor stub
}

SPHeader* SPSegment::getHeader(BufferFrame& bf){
	// returns a pointer to the header
	return (SPHeader*)bf.getData();
}

SPSlot* SPSegment::getSlots(BufferFrame& bf){
	SPHeader* header = getHeader(bf);
	return (SPSlot*)&(header[1]);
}

unsigned int SPSegment::numberOfFreeBytes(BufferFrame& bf){
	return (char*)(getHeader(bf)->dataStart) - (char*)(getHeader(bf)->firstFreeSlot);
}

void SPSegment::initializeSegmentPages(){
	// initializes SPHeaders of all pages
	for(int i = startPageID; i < (startPageID + length); i++){
		BufferFrame bf = bm.fixPage(i, true);

		SPHeader* header = getHeader(bf);
		header->firstFreeSlot = getSlots(bf);								// first free slot is right after header
		header->dataStart = (void*)((char*)bf.getData() + FRAME_SIZE -1);	// point to last byte in page
		header->slotCount = 0;

		bm.unfixPage(bf, true);
	}
}

} /* namespace std */
