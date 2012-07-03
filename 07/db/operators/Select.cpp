#include "Select.hpp"

namespace operators {

Select::Select(Operator* input, std::ostream& output) : output(output) {
	this->input = input;
}

Select::~Select() {

}

} /* namespace operators */
