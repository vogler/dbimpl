#include <iostream>

int main(){
	int myint[] = {1,2,3,4,5};

	for(int& i : myint){
		std::cout << i << std::endl;
	}
}
