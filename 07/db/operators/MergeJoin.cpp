#include "MergeJoin.hpp"

namespace operators {

MergeJoin::MergeJoin(Operator* input, std::ostream& output) : output(output) {
	this->input = input;
}

MergeJoin::~MergeJoin() {

}

} /* namespace operators */
