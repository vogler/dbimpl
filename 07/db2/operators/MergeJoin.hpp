#ifndef MERGEJOIN_HPP_
#define MERGEJOIN_HPP_

#include "Operator.hpp"
#include "../PlanReader/Plan.hpp"
#include <iostream>

namespace operators {

class MergeJoin : public Operator {
private:
	Operator* left;
	Operator* right;
	const std::vector<unsigned int>& attrLeft;
	const std::vector<unsigned int>& attrRight;
	const plan::Cmp cmp;
	bool doLeft;
	bool doRight;
	std::vector<Register*> buffer;
	enum state {START, LEFT, RIGHT, END};
public:
	MergeJoin(Operator* left, Operator* right,
			const std::vector<unsigned int>& attrLeft, const plan::Cmp& cmp, const std::vector<unsigned int>& attrRight)
		: left(left), right(right), attrLeft(attrLeft), attrRight(attrRight), cmp(cmp) {
		doLeft = true;
		doRight = true;
	}
	virtual ~MergeJoin();

	void open(){
		left->open();
		right->open();
	}

	bool eq(){
		std::vector<Register*> lreg = left->getOutput();
		std::vector<Register*> rreg = right->getOutput();
		for(unsigned int i=0; i<attrLeft.size(); i++) {
			if(cmp == plan::Cmp::EQ){
				if(lreg[attrLeft[i]] != rreg[attrRight[i]]) return false;
			}
		}
		return true;
	}

	bool lt(){
		std::vector<Register*> lreg = left->getOutput();
		std::vector<Register*> rreg = right->getOutput();
		for(unsigned int i=0; i<attrLeft.size(); i++) {
			if(cmp == plan::Cmp::EQ){
				if(!(lreg[attrLeft[i]] < rreg[attrRight[i]])) return false;
			}
		}
		return true;
	}

	/*function sortMerge(relation left, relation right, attribute a)
	     var relation output
	     var list left_sorted := sort(left, a) // Relation left sorted on attribute a
	     var list right_sorted := sort(right, a)
	     var attribute left_key, right_key
	     var set left_subset, right_subset // These sets discarded except where join predicate is satisfied
	     advance(left_subset, left_sorted, left_key, a)
	     advance(right_subset, right_sorted, right_key, a)
	     while not empty(left_subset) and not empty(right_subset)
	         if left_key = right_key // Join predicate satisfied
	             add cross product of left_subset and right_subset to output
	             advance(left_subset, left_sorted, left_key, a)
	             advance(right_subset, right_sorted, right_key, a)
	         else if left_key < right_key
	            advance(left_subset, left_sorted, left_key, a)
	         else // left_key > right_key
	            advance(right_subset, right_sorted, right_key, a)
	     return output*/
	bool next(){
		// check state
		if(state==state::START){
			return left->next() || right->next();
		}else if(state==state::END && buffer.empty()){ // at end of both sides and empty buffer -> done
			return false;
		}
		if(eq()){ // match
			if(doLeft && doRight) { // previous was also equal -> do cross product for n:m case
				cross++; // TODO save in list
			}
			doLeft = true; doRight = true; // advance on both sides
			return true;
		} else { // stopped being equal -> advance side that has still the old value

		}
		// TODO list !empty? -> emit n*m until cross==0

		if(lt()) { // left is smaller
			doLeft = true; doRight = false; cross = 0;
			return next(); // try next tuple
		} else { // right is smaller
			doLeft = false; doRight = true; cross = 0;
			return next(); // try next tuple
		}
		return false;
	}

	std::vector<Register*> getOutput() {
		// concatenate registers
		std::vector<Register*> lreg = left->getOutput();
		std::vector<Register*> rreg = right->getOutput();
		for(auto it = rreg.begin(); it != rreg.end(); ++it) {
			lreg.push_back(*it);
		}
		return lreg;
	}

	void close(){
		left->close();
		right->close();
	}
};

} /* namespace operators */
#endif /* MERGEJOIN_HPP_ */
