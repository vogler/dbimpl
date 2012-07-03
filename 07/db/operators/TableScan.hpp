#ifndef TABLESCAN_HPP_
#define TABLESCAN_HPP_

#include "../schema/SchemaSegment.hpp"
#include "../buffer/SegmentManager.hpp"
#include "Operator.hpp"

using namespace std;

namespace operators {

template <class T>
class TableScan : public Operator {
	unique_ptr<SPSegment> spSeg;
	TID tid;
	unique_ptr<T> currentTuple;
	std::vector<Register*>* reg;
public:
	TableScan(shared_ptr<SchemaSegment> scs, SegmentManager* sm, string relationName) {
		SegmentID segID = scs->getRelationSegmentID(relationName);
		this->spSeg = sm->getSegment<SPSegment>(segID);
		tid = 0;
		reg = new std::vector<Register*>;
	}
	~TableScan(){};

	void open(){
		tid = 0;
	}

	bool next(){
		currentTuple = spSeg->lookup<T>(tid);
		if(currentTuple==0){
			// check next page
			unsigned pageID = SlottedPage::getPageID(tid);
			tid = SlottedPage::getTID(pageID, 0);
			currentTuple = spSeg->lookup<T>(tid);
			if(currentTuple==0){ // no more tuple in relation
				return false;
			}
		}
		tid = SlottedPage::getTID(SlottedPage::getPageID(tid), SlottedPage::getSlotID(tid)+1);
		return true;
	}

	std::vector<Register*> getOutput(){
		Register* newRegister = new Register(sizeof(T));
		newRegister->setStructure(*currentTuple);
		if(reg->size() > 0){
			reg->pop_back();
		}
		reg->push_back(newRegister);
		return *this->reg;
	}

	void close(){

	}
};

} /* namespace operators */
#endif /* TABLESCAN_HPP_ */
