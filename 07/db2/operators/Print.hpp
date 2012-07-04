#ifndef PRINT_HPP_
#define PRINT_HPP_

#include "Operator.hpp"
#include <iostream>

namespace operators {

class Print : public Operator {
private:
	Operator* input;
	std::ostream& output;
public:
	Print(Operator* input, std::ostream& output);
	virtual ~Print();

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
#endif /* PRINT_HPP_ */
