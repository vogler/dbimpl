#ifndef SCANFILE_HPP_
#define SCANFILE_HPP_

#include "Operator.hpp"
#include <iostream>
#include <fstream>

namespace operators {

class ScanFile : public Operator {
private:
	std::fstream& input;
	std::vector<unsigned int> regSizes;
	std::vector<Register*> output;
public:
	ScanFile(std::fstream& input, std::vector<unsigned int>& regSizes) : input(input), regSizes(regSizes) {}
	virtual ~ScanFile();

	void open(){

	}

	bool next(){
		if(!input.eof()){
			output.clear();
			for(auto it = regSizes.begin(); it != regSizes.end(); ++it) {
				Register r(*it);
				input >> r.data;
				output.push_back(&r);
			}
			return true;
		} else {
			return false;
		}
	}

	std::vector<Register*> getOutput(){
		return output;
	}

	void close(){
		input.close();
	}
};

} /* namespace operators */
#endif /* SCANFILE_HPP_ */
