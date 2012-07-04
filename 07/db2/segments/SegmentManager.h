/*
 * SegmentManager.h
 *
 *  Created on: May 17, 2012
 *      Author: user
 */

#ifndef SEGMENTMANAGER_H_
#define SEGMENTMANAGER_H_

#include <memory>
#include <assert.h>
#include "datastructures.h"
#include "SPSegment.h"
#include "../buffer/BufferManager.h"

namespace std {

class SegmentManager {
private:
	BufferManager& bm;
	SIMetadata* getSegmentInventoryMetadata(BufferFrame& bf);
	SIEntry* getSegments(BufferFrame& bf);
public:
	SegmentManager(BufferManager& bm);
	virtual ~SegmentManager();

	template <class T>
	SegmentID create(unsigned int size){
		// creates new segment with "size" pages and returns its SegmentID

		// get the segment inventory from page 0
		BufferFrame bf = this->bm.fixPage(0, true);

		// get next available SegmentID
		SIMetadata* metadata = getSegmentInventoryMetadata(bf);
		SegmentID SID = metadata->numberOfSegments;

		// assert that page which holds segment inventory has enough space for another segment entry
		assert(SID*sizeof(SIEntry) + 1 < 4096-sizeof(SIEntry));

		// add segment information entry
		SIEntry* segments = getSegments(bf);
		segments[SID].startPageID = metadata->firstFreePage;
		segments[SID].length = size;

		// update metadata
		metadata->firstFreePage += size;
		metadata->numberOfSegments++;

		// initialize the segment pages
		//SPSegment* segment = new SPSegment(segments[SID], this->bm);
		//segment->initializeSegmentPages();
		//delete segment;
		T* segment = new T(segments[SID], this->bm);
		segment->initializeSegmentPages();
		delete segment;

		// unfix page with segment inventory
		this->bm.unfixPage(bf, true);

		return SID;
	}


	// returns the segment defined by this SegmentID
	template <class T>
	unique_ptr<T> get(SegmentID SID){
		// get the segment inventory from page 0
		BufferFrame bf = this->bm.fixPage(0, false);

		// assert that this segment exists
		assert(SID < getSegmentInventoryMetadata(bf)->numberOfSegments);

		// check if segment exists
		// TODO: this has to be more sophisticated if deletion of segments is allowed
		if(SID < getSegmentInventoryMetadata(bf)->numberOfSegments){
			SIEntry entry = getSegments(bf)[SID];							// copy the segment information
			this->bm.unfixPage(bf, false);									// unfix segment inventory page
			return unique_ptr<T> (new T(entry, this->bm));					// return slotted page segment
		} else {
			// if segment doesnt exist, unfix page, show error, and return null
			this->bm.unfixPage(bf, false);
			cerr << "Segment with id " << SID << " not found!" << endl;
			return NULL;
		}
	}

	// custom methods
	unsigned int getNumberOfSegments();			// returns the number of segments
	void reinitializeSegmentInventory();		// deletes current information about segments, thus reinitializing the database
};

} /* namespace std */
#endif /* SEGMENTMANAGER_H_ */
