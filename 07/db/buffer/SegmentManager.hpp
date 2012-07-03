#ifndef SEGEMENTMANAGER_HPP_
#define SEGEMENTMANAGER_HPP_

#include <iostream>
#include "BufferManager.hpp"
#include "BufferFrame.hpp"
#include "SISegment.hpp"
#include "SPSegment.hpp"
#include <vector>

using namespace std;

class SISegment;


/**
 * Segment Manager
 */
class SegmentManager
{


public:
	static class IllegalSegmentException: public exception
	{
	} IllegalSegmentException;

	/** public member **/


	/** public methods **/
	SegmentManager(BufferManager& bm);
	~SegmentManager();
	
	/**
	 * create new segment
	 */
	template <class T>
	SegmentID createSegment(Segment::SegmentType type, unsigned size){
		return this->si->createSegment(type, size);
	}
	/**
	 * get segment from id
	 */
	template <class T>
	unique_ptr<T> getSegment(SegmentID id){
		SISegment::SegmentEntry entry = this->si->segmentEntry(id);

		vector<BufferFrame*> pages;
		for (unsigned i=0; i<entry.size; i++) {
			pages.push_back(&bm.fixPage(entry.pages[i], false));
		}

		switch (entry.type) {
			case Segment::SegmentType::SP:
				return unique_ptr<T> (new SPSegment(bm, pages));
				break;
			case Segment::SegmentType::Schema:
				return unique_ptr<T> (new SchemaSegment(entry, bm));
				break;
			default:
				return NULL;
			break;
		}
	}
	/**
	 * get size in pages
	 */
	unsigned size();

protected:
	BufferManager& bm;
	SISegment* si;
};


#endif /* SEGEMENTMANAGER_HPP_ */
