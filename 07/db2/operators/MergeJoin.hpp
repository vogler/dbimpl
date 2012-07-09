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
	std::vector<Register*> leftPrev;
	std::vector<Register*> lreg;
	std::vector<Register*> rreg;
	const std::vector<unsigned int>& attrLeft;
	const std::vector<unsigned int>& attrRight;
	const plan::Cmp cmp;
	bool doLeft;
	bool doRight;
	std::vector<std::vector<Register*>> buffer;
	std::vector<std::vector<Register*>> workBuffer;
	enum tstate {START, LEFT, RIGHT} state;
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

	bool eq(std::vector<Register*> a, std::vector<Register*> b){
		for(unsigned int i=0; i<attrLeft.size(); i++) {
			if(cmp == plan::Cmp::EQ){
				if(a[attrLeft[i]] != b[attrRight[i]]) return false;
			}
		}
		return true;
	}
	bool eq(){
		return eq(left->getOutput(), right->getOutput());
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
	while not at the end of either input or buffer not empty
		begin
			// if R1 is duplicate -> playback buffer until empty
			// else -> discard buffer
			// if left at end -> done, else buffer refill if R1 duplicate
			// if right at end -> advance left to check for buffer
			if R1 joins with R2
				begin
					// buffer.add(R2)
					yield (R1, R2)
					get next row R2 from input 2
				end
			else if R1 < R2
				get next row R1 from input 1
			else
				get next row R2 from input 2
		end*/
	bool next(){
		// check state
		if(state==tstate::START){
			state = tstate::RIGHT;
			bool r = left->next() | right->next();
			lreg = left->getOutput();
			rreg = right->getOutput();
			return r;
		}
		// check buffer
		if(eq(left->getOutput(), leftPrev) && !workBuffer.empty()){
			rreg = workBuffer.front();
			workBuffer.erase(workBuffer.begin());
			return true;
		}else{
			workBuffer.clear();
			buffer.clear();
		}
		// advance one side
		if(state==tstate::LEFT){
			leftPrev = left->getOutput(); // save previous values
			if(!left->next()) return false; // done
			lreg = left->getOutput();
			// check for buffer replay
			if(eq(left->getOutput(), leftPrev) && !buffer.empty()){
				workBuffer = buffer; // fill in all cached values from the right side into working list
				return next();
			}
		}
		if(state==tstate::RIGHT){
			bool r = right->next();
			rreg = right->getOutput();
			if(!r){ // right side at end -> advance left to check if we have to replay the buffer
				state = tstate::LEFT;
				return next();
			}
		}

		if(eq()){ // match
			buffer.push_back(right->getOutput()); // save right side in playback buffer
			state = tstate::RIGHT; // advance right side in the next step
			return true;
		}
		// not equal

		if(lt()) { // left is smaller
			state = tstate::LEFT; // advance left side in the next step
			return next();
		} else { // right is smaller
			state = tstate::RIGHT; // advance right side in the next step
			return next();
		}
		return false;
	}

	std::vector<Register*> getOutput() {
		// concatenate registers
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
