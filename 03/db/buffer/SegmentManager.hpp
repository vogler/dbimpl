#ifndef SEGEMENTMANAGER_HPP_
#define SEGEMENTMANAGER_HPP_

#include <iostream>
#include "BufferManager.hpp"
#include "BufferFrame.hpp"
#include "SPSegment.hpp"
#include <vector>

using namespace std;

class SegmentManager
{
public:
	/** public member **/


	/** public methods **/
	SegmentManager(BufferManager& bm);
	~SegmentManager();
	
	unsigned addSegment();
	SPSegment& getSegment(unsigned id);
	unsigned size();

protected:
	BufferManager& bm;
	BufferFrame& si;
	vector<SPSegment*> segments;
};


#endif /* SEGEMENTMANAGER_HPP_ */
