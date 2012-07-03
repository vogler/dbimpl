#ifndef SELECT_HPP_
#define SELECT_HPP_

#include "Operator.hpp"
#include <iostream>

namespace operators {

class Select : public Operator {
private:
	Operator* input;
	std::ostream& output;
public:
	Select(Operator* input, std::ostream& output);
	virtual ~Select();

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
#endif /* SELECT_HPP_ */
