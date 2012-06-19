#include "Register.hpp"
#include <assert.h>
#include <string.h>
#include <sstream>
#include <iostream>

using namespace std;

namespace operators {

Register::Register(unsigned int sizeInBytes) {
	this->size = sizeInBytes;
	this->data = new char[sizeInBytes];
}

Register::Register() {
	this->size = 4;
	this->data = new char[this->size];
}

Register::~Register() {

}

int Register::getInteger(unsigned int offset) {
	int* x = reinterpret_cast<int*>(this->data + offset);
	return *x;
}
void Register::setInteger(unsigned int offset, int value) {
	offsetInReg[offset] = Integer;
	reinterpret_cast<int*>(this->data + offset)[0] = value;
}
std::string Register::getString(unsigned int offset) {
	return std::string(this->data + offset + 4,
			(unsigned int) this->data[offset]);
}
void Register::setString(unsigned int offset, const std::string& value) {
	assert(value.length() <= (this->size-offset-4));
	offsetInReg[offset] = String;
	reinterpret_cast<unsigned int*>(this->data + offset)[0] = value.length();
	memcpy(this->data + offset + 4, value.c_str(), value.length());
}

void Register::setSize(unsigned int sizeInBytes) {
	delete[] this->data;
	this->size = sizeInBytes;
	this->data = new char[sizeInBytes];
}

std::string Register::toString() {
	std::map<unsigned int, DataType>::iterator iter;
	std::string strToReturn = "";

	for (iter = offsetInReg.begin(); iter != offsetInReg.end(); ++iter) {
		if (iter->second == Integer) {
			std::stringstream ss;
			ss << getInteger(iter->first);
			strToReturn += ss.str();
		} else if (iter->second == String) {
			strToReturn += getString(iter->first);
		}
		strToReturn += ", ";
	}

	return strToReturn;
}

DataType Register::getDataType() {
	return offsetInReg[0];
}

int Register::getHash() {
	int result = 0;
	for (unsigned int i = 0; i < size; i += 4) {
		result += getInteger(i);
	}
	return result;
}

bool Register::operator<(Register* other) {
	assert(other->getDataType()==this->getDataType());
	if (this->getDataType() == Integer) {
		return this->getInteger() < other->getInteger();
	} else if (this->getDataType() == String) {
		return this->getString().compare(other->getString()) < 0;
	} else {
		cerr << "cannot compare types" << endl;
		assert(false);
	}
	return false;
}
bool Register::operator==(Register* other) {
	assert(other->getDataType()==this->getDataType());
	if (this->getDataType() == Integer) {
		return this->getInteger() == other->getInteger();
	} else if (this->getDataType() == String) {
		return this->getString().compare(other->getString()) == 0;
	} else {
		cerr << "cannot compare types" << endl;
		assert(false);
	}
	return false;
}

} /* namespace operators */
