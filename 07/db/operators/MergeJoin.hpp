#ifndef MERGEJOIN_HPP_
#define MERGEJOIN_HPP_

#include "Operator.hpp"
#include <iostream>

namespace operators {

class MergeJoin : public Operator {
private:
	Operator* input;
	std::ostream& output;
public:
	MergeJoin(Operator* input, std::ostream& output);
	virtual ~MergeJoin();

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
#endif /* MERGEJOIN_HPP_ */
