/*
 * BPlusTreeSegment.cpp
 *
 *  Created on: May 24, 2012
 *      Author: user
 */

#include "BPlusTreeSegment.h"
#include <assert.h>

namespace std {

BPlusTreeSegment::BPlusTreeSegment(SIEntry siEntry, BufferManager& bm) : SPSegment(siEntry, bm) {
}

BPlusTreeSegment::~BPlusTreeSegment() {
	// TODO Auto-generated destructor stub
}

bool BPlusTreeSegment::isLeaf(SNodeReference ref){
	// check if pageID is within allowed range
	if(ref.pageID > this->length){
		return NULL;
	}

	// calculate overall page id
	unsigned int overallPageID = ref.pageID + this->startPageID;
	// get frame
	BufferFrame bf = this->bm.fixPage(overallPageID, false);

	// check if wrong slotID or record was deleted
	if(this->getHeader(bf)->slotCount < ref.slotID
			|| this->getSlots(bf)[ref.slotID].offset == NULL){
		this->bm.unfixPage(bf, false);
		return NULL;
	} else {
		SNodeHeader* nodeHeader = (SNodeHeader*)this->getSlots(bf)[ref.slotID].offset;
		bool result = false;
		if(nodeHeader->isLeafNode)
			result = true;
		this->bm.unfixPage(bf, false);
		return result;
	}
}

} /* namespace std */
