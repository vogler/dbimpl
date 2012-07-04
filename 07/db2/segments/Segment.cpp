/*
 * Segment.cpp
 *
 *  Created on: May 22, 2012
 *      Author: user
 */

#include "Segment.h"

namespace std {

Segment::Segment(SIEntry siEntry, BufferManager& bm) : bm(bm) {
	this->startPageID = siEntry.startPageID;
	this->length = siEntry.length;
}

Segment::~Segment() {
	// TODO Auto-generated destructor stub
}

void Segment::initializeSegmentPages(){
	// no initialization logic required
}

unsigned int Segment::getSize(){
	return this->length;
}

unsigned int Segment::getGlobalPageID(unsigned int localPageID){
	if(localPageID>=length){
		cerr << "localPageID is out of range" << endl;
		throw;
	}
	else {
		return this->startPageID + localPageID;
	}
}

} /* namespace std */
