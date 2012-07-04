/*
 * SPSegment.h
 *
 *  Created on: May 17, 2012
 *      Author: user
 */

#ifndef SPSEGMENT_H_
#define SPSEGMENT_H_

#include <memory>
#include "datastructures.h"
#include "../buffer/BufferFrame.h"
#include "../buffer/BufferManager.h"

#include "Segment.h"

namespace std {

class SPSegment : public Segment {
protected:
	SPHeader* getHeader(BufferFrame& bf);				// returns a pointer to the header
	SPSlot* getSlots(BufferFrame& bf);					// returns pointer to first slot (starts after header)
	unsigned int numberOfFreeBytes(BufferFrame& bf);	// returns number of free bytes between end of slots and first record
public:
	SPSegment(SIEntry siEntry, BufferManager& bm);
	virtual ~SPSegment();

	template <class T>
	TID insert(const T& r){
		// find page with enough space
		BufferFrame bf;
		SPHeader *header;
		unsigned int page = 0;
		for(unsigned int i=this->startPageID; i < this->startPageID + this->length; i++){
			bf = this->bm.fixPage(i, true);
			header = getHeader(bf);
			if(numberOfFreeBytes(bf) > sizeof(T) + sizeof(SPSlot)){
				page = i;
				break;	// page will be unfixed in next if scope
			}
			this->bm.unfixPage(bf, true);
		}

		TID result;

		if(page!=0){
			header->dataStart = (void*)((T*)header->dataStart - 1);		// set dataStart one T-position before dataStart
			*((T*)header->dataStart) = r;								// write record data

			*header->firstFreeSlot = { header->dataStart, sizeof(T) };	// write slot entry
			++ header->firstFreeSlot;									// increment pointer to first free slot
			result = { header->slotCount, page - this->startPageID };	// define TID
			++ header->slotCount;										// increment slot counter

			this->bm.unfixPage(bf, true);								// unfix page
		} else {
			cerr << "not enough space to insert a new record" << endl;
			result = { 0, 0 };	// return TID with slot and page id 0 as record could not be saved
		}

		return result;
	}

	template <class T>
	unique_ptr<T> lookup(TID tid){
		// check if pageID is within allowed range
		if(tid.pageID > this->length){
			return NULL;
		}

		// calculate overall page id
		unsigned int overallPageID = tid.pageID + this->startPageID;
		// get frame
		BufferFrame bf = this->bm.fixPage(overallPageID, false);

		// check if wrong slotID or record was deleted
		if(this->getHeader(bf)->slotCount < tid.slotID
				|| this->getSlots(bf)[tid.slotID].offset == NULL){
			this->bm.unfixPage(bf, false);
			return NULL;
		} else {
			unique_ptr<T> result (new T);
			*result = *(T*)this->getSlots(bf)[tid.slotID].offset;
			this->bm.unfixPage(bf, false);
			return result;
		}
	}

	// TODO: check to collapse the following two functions
	template <class T>
	bool update(TID tid, unique_ptr<T> r){
		// check if pageID is within allowed range
		if(tid.pageID > this->length){
			return false;
		}

		// calculate overall page id
		unsigned int overallPageID = tid.pageID + this->startPageID;
		// get frame
		BufferFrame bf = this->bm.fixPage(overallPageID, true);

		// check if wrong slotID or record was deleted
		if(this->getHeader(bf)->slotCount < tid.slotID
				|| this->getSlots(bf)[tid.slotID].offset == NULL){
			this->bm.unfixPage(bf, false);
			return false;
		} else {
			*(T*)this->getSlots(bf)[tid.slotID].offset = *r;
			this->bm.unfixPage(bf, true);
			return true;
		}
	}
	template <class T>
	bool update(TID tid, T r){
		// check if pageID is within allowed range
		if(tid.pageID > this->length){
			return false;
		}

		// calculate overall page id
		unsigned int overallPageID = tid.pageID + this->startPageID;
		// get frame
		BufferFrame bf = this->bm.fixPage(overallPageID, true);

		// check if wrong slotID or record was deleted
		if(this->getHeader(bf)->slotCount < tid.slotID
				|| this->getSlots(bf)[tid.slotID].offset == NULL){
			this->bm.unfixPage(bf, false);
			return false;
		} else {
			*(T*)this->getSlots(bf)[tid.slotID].offset = r;
			this->bm.unfixPage(bf, true);
			return true;
		}
	}

	// TODO: more complex implementation to correctly release the memory
	// pretty simple implementation of delete, just invalidating the slot by setting offset and length to zero
	bool del(TID tid){
		if(tid.pageID > this->length){
			return false;
		}
		// calculate overall page id
		unsigned int overallPageID = tid.pageID + this->startPageID;
		// get frame
		BufferFrame bf = this->bm.fixPage(overallPageID, true);

		// check if wrong slotID or record was deleted
		if(this->getHeader(bf)->slotCount < tid.slotID
				|| this->getSlots(bf)[tid.slotID].offset == NULL){
			this->bm.unfixPage(bf, false);
			return false;
		} else {
			this->getSlots(bf)[tid.slotID].offset = NULL;
			this->getSlots(bf)[tid.slotID].length = 0;
			this->bm.unfixPage(bf, true);
			return true;
		}
	}

	virtual void initializeSegmentPages();			// initializes page headers
};

} /* namespace std */
#endif /* SPSEGMENT_H_ */
