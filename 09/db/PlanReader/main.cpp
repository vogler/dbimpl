#include <iostream>

#include "Plan.hpp"

using plan::Plan;

int main(int argc, char* argv[]) {
   if (argc != 2)
      return -1;
   Plan p;
   p.fromFile(argv[1]);
   p.print(std::cout);
   return 0;
}
