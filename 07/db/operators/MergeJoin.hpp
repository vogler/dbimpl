#ifndef MERGEJOIN_HPP_
#define MERGEJOIN_HPP_

#include "Operator.hpp"
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
	int cross;
public:
	MergeJoin(Operator* left, Operator* right,
			const std::vector<unsigned int>& attrLeft, const plan::Cmp& cmp, const std::vector<unsigned int>& attrRight)
		: left(left), right(right), attrLeft(attrLeft), attrRight(attrRight), cmp(cmp) {
		doLeft = true;
		doRight = true;
		cross = 0;
	}
	virtual ~MergeJoin();

	void open(){
		left->open();
		right->open();
	}

	bool equal(){
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

	bool next(){
		if(doLeft){
			if(!left->next()) return false; // through with left
		}
		if(doRight){
			if(!right->next()) return false; // through with right
		}
		if(equal()){ // match
			if(doLeft && doRight) { // do cross product for n:m case
				cross++;
			}
			doLeft = true; doRight = true; // advance on both sides
			return true;
		} else { // stopped being equal -> advance side that has still the old value

		}
		// TODO emit n*m until cross==0

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
