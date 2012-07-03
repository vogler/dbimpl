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
	const std::vector<unsigned int> attributeIds;
	plan::Sort::Order order;
	bool cmp(std::vector<Register*> i, std::vector<Register*> j){
		bool asc = order==plan::Sort::Order::Asc;
		for(auto it = attributeIds.begin(); it != attributeIds.end(); ++it) {
			if(asc && i[*it]>=j[*it]) return false;
			if(!asc && i[*it]<j[*it]) return false;
		}
		return true;
	}
public:
	Sort(Operator* input, const std::vector<unsigned int> attributeIds, plan::Sort::Order order);
	virtual ~Sort();



	void open(){
		input->open();
		std::fstream tmpFile("tmp/sort"); // TODO multiple simultaneous sorts?
		std::vector<unsigned int> regSizes; // save sizes of registers for reconstruction
		bool init = true;
		// pipeline breaker -> read all input tuples and write them to a temporary file
		// better solution: read chunks of tuples and sort them in memory, then write runs to disk, then merge them
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
//		externalSort(tmpFile, regSizes, cmp);

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
