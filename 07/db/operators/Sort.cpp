#include "Sort.hpp"

namespace operators {

Sort::Sort(Operator* input, std::vector<unsigned int> attributeIds, plan::Sort::Order order) {
	this->input = input;
	this->attributeIds = attributeIds;
	this->order = order;
}

Sort::~Sort() {

}

} /* namespace operators */
