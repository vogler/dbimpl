/*
 * datastructures.h
 *
 *  Created on: May 17, 2012
 *      Author: user
 */

#ifndef SEGMENTS_DATASTRUCTURES_H_
#define SEGMENTS_DATASTRUCTURES_H_

using namespace std;

typedef unsigned int PageID;

struct TID {
	unsigned short int slotID;	// 2 byte for slotID
	PageID pageID;				// 4 byte for pageID, relative to the segments start page
};

// this is a bad choice due to alignment (requires 8 byte instead of 6)
struct SPSlot {
	void* offset;				// start of data item, NULL if item was deleted
	unsigned short int length;	// length in bytes of data item
};

struct SPHeader {
	//unsigned int LSN;
	unsigned short int slotCount;
	SPSlot* firstFreeSlot;
	void* dataStart;		// lower end of data
	//unsigned int freeSpace;
};

struct SlottedPage {
	SPHeader header;
	SPSlot slots[1];
};

typedef unsigned int SegmentID;

// represents an entry in the segment inventory's hash map
struct SIEntry {
	//SegmentID SID;				// id of the segment
	PageID startPageID;	// pageID of first page
	unsigned int length;		// number of pages
};

struct SIMetadata {
	unsigned int numberOfSegments;
	unsigned int firstFreePage;
};

struct Record {
	int test;
};
struct RecordBig {
	int test;
	int test1;
};

#endif /* DATASTRUCTURES_H_ */
