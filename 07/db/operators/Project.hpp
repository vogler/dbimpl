#ifndef PROJECT_HPP_
#define PROJECT_HPP_

#include "Operator.hpp"
#include <iostream>

namespace operators {

class Project : public Operator {
private:
	Operator* input;
	std::ostream& output;
public:
	Project(Operator* input, std::ostream& output);
	virtual ~Project();

	void open(){
		input->open();
	}

	bool next(){
		if(input->next()){
			std::vector<Register*> reg = input->getOutput();
			for(auto it = reg.begin(); it != reg.end(); ++it) {
				output << (*it)->toString() << std::endl;
			}
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
#endif /* PROJECT_HPP_ */
