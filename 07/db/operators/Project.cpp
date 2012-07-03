#include "Project.hpp"

namespace operators {

Project::Project(Operator* input, std::ostream& output) : output(output) {
	this->input = input;
}

Project::~Project() {

}

} /* namespace operators */
