#ifndef SISEGMENT_HPP_
#define SISEGMENT_HPP_

#include <iostream>
#include "BufferFrame.hpp"
#include "Record.hpp"
#include "Segment.hpp"
#include "SegmentManager.hpp"
#include <vector>
#include <map>

using namespace std;

class SISegment: public Segment
{
public:

	class SegmentEntry {
	public:
		SegmentType type;
		unsigned size;
		vector<unsigned> pages;

		SegmentEntry(SegmentType type, unsigned size, vector<unsigned> pages): type(type), size(size), pages(pages) {
		}
	};

	SISegment(BufferManager& bm, BufferFrame* page);

	SegmentEntry segmentEntry(SegmentID id);
	SegmentID createSegment(Segment::SegmentType type, unsigned size);
	unsigned pageCount();



protected:
	vector<SegmentEntry> _segments;
	unsigned _pageCount;
};


#endif /* SISEGMENT_HPP_ */
