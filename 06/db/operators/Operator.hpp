#ifndef OPERATOR_HPP_
#define OPERATOR_HPP_

#include <vector>
#include "Register.hpp"

namespace operators {

class Operator {
public:
	Operator();
	virtual ~Operator();

	virtual void open() = 0;
	virtual bool next() = 0;
	virtual std::vector<Register*> getOutput() = 0;
	virtual void close() = 0;
};

} /* namespace operators */
#endif /* OPERATOR_HPP_ */
