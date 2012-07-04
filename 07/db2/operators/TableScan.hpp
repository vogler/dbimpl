#ifndef TABLESCAN_HPP_
#define TABLESCAN_HPP_

#include "../schema/SchemaSegment.h"
#include "../segments/datastructures.h"
#include "Operator.hpp"

namespace operators {

template <class T>
class TableScan : public Operator {
	unique_ptr<SPSegment> spSeg;
	TID currentTupleTID;
	unique_ptr<T> currentTuple;
	std::vector<Register*>* reg;
public:
	TableScan(shared_ptr<SchemaSegment> scs, SegmentManager* sm, string relationName) {
		SegmentID segID = scs->getRelationSegmentID(relationName);
		this->spSeg = sm->get<SPSegment>(segID);
		currentTupleTID = {0,0};
		//reg = new Register(sizeof(T));
		reg = new std::vector<Register*>;
	}
	~TableScan(){};

	void open(){
		currentTupleTID = {0,0};
	}

	bool next(){
		currentTuple = spSeg->lookup<T>(currentTupleTID);
		if(currentTuple==0){
			// check next page
			currentTupleTID.pageID++;
			currentTupleTID.slotID = 0;
			currentTuple = spSeg->lookup<T>(currentTupleTID);
			if(currentTuple==0){ // no more tuple in relation
				return false;
			}
		}
		currentTupleTID.slotID++;
		return true;
	}

	// Get all produced values
	std::vector<Register*> getOutput(){
		Register* newRegister = new Register(sizeof(T));
		newRegister->setStructure(*currentTuple);
		if(reg->size()>0)
			reg->pop_back();	// delete last value
		reg->push_back(newRegister);
		return *this->reg;
	}

	void close(){
		// ?
	}
};

//#include "TableScan.tpp"

} /* namespace operators */
#endif /* TABLESCAN_HPP_ */
