#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>
#include <unordered_map>

#include "../PlanReader/Plan.hpp"

using namespace std;
using plan::Plan;

int main(int argc, char** argv) {
	// Check arguments
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " <planFile>" << endl;
//      return -1;
	}
	Plan p;
	p.fromFile(argv[1]);
	p.print(std::cout);
	const plan::Operator root = p.getRoot();
	// TODO: walk down tree and execute the corresponding physical operators


	return 0;
}
