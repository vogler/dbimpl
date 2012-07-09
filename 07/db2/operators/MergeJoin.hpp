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
	std::vector<std::vector<Register*>> buffer;
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

	/*get first row R1 from input 1
	get first row R2 from input 2
	while not at the end of either input
		begin
			if R1 joins with R2
				begin
					// buffer.add(R2)
					return (R1, R2)
					get next row R2 from input 2
				end
			else if R1 < R2
				// R1 duplicate -> playback buffer until empty
				// else -> discard buffer
				get next row R1 from input 1
			else
				get next row R2 from input 2
		end*/
	bool next(){
		// check state
		if(state==state::START){
			return left->next() || right->next();
		}else if(state==state::END && buffer.empty()){ // at end of both sides and empty buffer -> done
			return false;
		}
		// advance one side and return if at end, TODO buffer?
		if(state==state::LEFT){
			if(!left->next()) return false;
		}
		if(state==state::RIGHT){
			if(!right->next()) return false;
		}

		if(eq()){ // match
			buffer.push_back(right->getOutput()); // save right side in playback buffer
			state = state::RIGHT; // advance right side in the next step
			return true;
		}
		// not equal

		if(lt()) { // left is smaller
			state = state::LEFT; // advance left side in the next step
			return next();
		} else { // right is smaller
			state = state::RIGHT; // advance right side in the next step
			return next();
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
