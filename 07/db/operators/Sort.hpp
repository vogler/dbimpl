#ifndef SORT_HPP_
#define SORT_HPP_

#include "Operator.hpp"
#include "ScanFile.hpp"
#include <iostream>
#include <fstream>

namespace operators {

class Sort : public Operator {
private:
	Operator* input;
	Operator* sorted;
	std::vector<unsigned int> attributeIds;
	plan::Sort::Order order;
public:
	Sort(Operator* input, std::vector<unsigned int> attributeIds, plan::Sort::Order order);
	virtual ~Sort();

	void open(){
		input->open();
		std::fstream tmpFile("tmp/sort");
		std::vector<unsigned int> regSizes; // save sizes of registers for reconstruction
		bool init = true;
		// pipeline breaker -> read all input tuples and write them to a temporary file
		while(input->next()){
			std::vector<Register*> reg = input->getOutput();
			for(auto it = reg.begin(); it != reg.end(); ++it) {
				if(init){
					regSizes.push_back(sizeof(*it));
				}
				// write register to file
				tmpFile << (*it);
			}
			init = false;
		}

		// TODO do external sort using attributeIds -> sorting/sort.cpp
//		externalSort(tmpFile, regSizes, attributeIds, order);

		// open TableScan on file of sorted data
		sorted = new ScanFile(tmpFile, regSizes);
	}

	bool next(){
		return sorted->next();
	}

	std::vector<Register*> getOutput(){
		return sorted->getOutput();
	}

	void close(){
		sorted->close();
		input->close();
	}
};

} /* namespace operators */
#endif /* SORT_HPP_ */
