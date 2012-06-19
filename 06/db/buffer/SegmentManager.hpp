#ifndef SEGEMENTMANAGER_HPP_
#define SEGEMENTMANAGER_HPP_

#include <iostream>
#include "BufferManager.hpp"
#include "BufferFrame.hpp"
#include "SISegment.hpp"
#include "SPSegment.hpp"
#include "BTreeSegment.hpp"
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
	SegmentID createSegment(Segment::SegmentType type, unsigned size);
	/**
	 * get segment from id
	 */
	Segment& getSegment(SegmentID id);
	/**
	 * get size in pages
	 */
	unsigned size();

protected:
	BufferManager& bm;
	SISegment* si;
};


#endif /* SEGEMENTMANAGER_HPP_ */
