#include "Print.hpp"

namespace operators {

Print::Print(Operator* input, std::ostream& output) : output(output) {
	this->input = input;
}

Print::~Print() {

}

} /* namespace operators */
