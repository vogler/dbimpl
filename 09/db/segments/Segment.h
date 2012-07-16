/*
 * Segment.h
 *
 *  Created on: May 22, 2012
 *      Author: user
 */

#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "../buffer/BufferManager.h"
#include "datastructures.h"

namespace std {

class Segment {
protected:
	unsigned int startPageID;							// pageID of first page
	unsigned int length;								// number of pages
	BufferManager& bm;
public:
	Segment(SIEntry siEntry, BufferManager& bm);
	virtual ~Segment();

	virtual void initializeSegmentPages();
	unsigned int getSize();
	unsigned int getGlobalPageID(unsigned int localPageID);
};

} /* namespace std */
#endif /* SEGMENT_H_ */
