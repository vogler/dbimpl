/*
 * SegmentManager.cpp
 *
 *  Created on: May 17, 2012
 *      Author: user
 */

#include "SegmentManager.h"

namespace std {

SegmentManager::SegmentManager(BufferManager& bm) : bm(bm) {
	// TODO Auto-generated constructor stub
	//this->bm = bm;
}

SegmentManager::~SegmentManager() {
	// TODO Auto-generated destructor stub
}

SIMetadata* SegmentManager::getSegmentInventoryMetadata(BufferFrame& bf){
	return (SIMetadata*)bf.getData();
}
SIEntry* SegmentManager::getSegments(BufferFrame& bf){
	SIMetadata *i = getSegmentInventoryMetadata(bf);
	return (SIEntry*)&(i[1]);
}

unsigned int SegmentManager::getNumberOfSegments(){
	BufferFrame bf = this->bm.fixPage(0, true);
	int number = this->getSegmentInventoryMetadata(bf)->numberOfSegments;
	this->bm.unfixPage(bf, false);
	return number;
}

void SegmentManager::reinitializeSegmentInventory(){
	BufferFrame bf = this->bm.fixPage(0, true);

	SIMetadata* metadata = getSegmentInventoryMetadata(bf);
	metadata->firstFreePage = 1;
	metadata->numberOfSegments = 0;

	this->bm.unfixPage(bf, true);
}

} /* namespace std */
