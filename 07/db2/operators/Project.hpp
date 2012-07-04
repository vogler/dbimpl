#ifndef PROJECT_HPP_
#define PROJECT_HPP_

#include "Operator.hpp"
#include <iostream>

namespace operators {

class Project : public Operator {
private:
	Operator* input;
	const std::vector<unsigned int>& attributeIds;
public:
	Project(Operator* input, const std::vector<unsigned int>& attributeIds) : input(input), attributeIds(attributeIds){}
	virtual ~Project();

	void open(){
		input->open();
	}

	bool next(){
		return input->next();
	}

	std::vector<Register*> getOutput(){
		std::vector<Register*> reg = input->getOutput();
		std::vector<Register*> proj;
		for(auto it = attributeIds.begin(); it != attributeIds.end(); ++it) {
			proj.push_back(reg[*it]);
		}
		return proj;
	}

	void close(){
		input->close();
	}
};

} /* namespace operators */
#endif /* PROJECT_HPP_ */
