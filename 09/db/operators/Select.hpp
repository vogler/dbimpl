#ifndef SELECT_HPP_
#define SELECT_HPP_

#include "Operator.hpp"
#include "../PlanReader/Plan.hpp"
#include <iostream>

namespace operators {

class Select : public Operator {
private:
	Operator* input;
	const std::vector<unsigned int>& attributeIds;
	const plan::Cmp cmp;
	const std::vector<std::string>& constants;
public:
	Select(Operator* input, const std::vector<unsigned int>& attributeIds,
			const plan::Cmp cmp,
			const std::vector<std::string>& constants) : input(input), attributeIds(attributeIds), cmp(cmp), constants(constants){}
	virtual ~Select();

	void open(){
		input->open();
	}

	bool next(){
		if(input->next()){
			std::vector<Register*> reg = input->getOutput();
			for(auto it = attributeIds.begin(); it != attributeIds.end(); ++it) {
				unsigned int i = *it;
				if(cmp == plan::Cmp::EQ && reg[i]->getString() != constants[i]){ // tuple in selection?
					return next(); // -> try next one
				}
			}
			// no return yet -> all attributes equal the constants -> keep tuple
			return true;
		} else {
			return false;
		}
	}

	std::vector<Register*> getOutput(){
		return input->getOutput();
	}

	void close(){
		input->close();
	}
};

} /* namespace operators */
#endif /* SELECT_HPP_ */
