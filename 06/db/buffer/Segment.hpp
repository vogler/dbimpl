/*
 * Segment.hpp
 *
 *  Created on: May 29, 2012
 *      Author: user
 */

#ifndef SEGMENT_HPP_
#define SEGMENT_HPP_


#include "BufferFrame.hpp"
#include "BufferManager.hpp"
#include <vector>
using namespace std;

class BufferManager;


typedef unsigned SegmentID;

/**
 * Basic Segment
 */
class Segment
{
public:
	enum SegmentType {
		SI,
		SP,
		BTree,
		Schema
	};


	Segment(BufferManager& bm, vector<BufferFrame*> pages);
	virtual ~Segment();

	/**
	 * size in pages
	 */
	unsigned size();

	/**
	 * get certain page
	 */
	BufferFrame& getPage(SegmentID id);

//protected:
	BufferManager& bm;
	vector<BufferFrame*> pages;
};


#endif /* SEGMENT_HPP_ */
