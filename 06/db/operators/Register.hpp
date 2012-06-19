#ifndef REGISTER_HPP_
#define REGISTER_HPP_

#include <string>
#include <map>

namespace operators {

enum DataType {
	Integer, String, Struct
};

class Register {
	unsigned int size;
	char *data;
	std::map<unsigned int, DataType> offsetInReg;
private:
	int getInteger(unsigned int offset);
	void setInteger(unsigned int offset, int value);
	std::string getString(unsigned int offset);
	void setString(unsigned int offset, const std::string& value);

	template<class T>
	void setStructure(unsigned int offset, T value) {
		offsetInReg[offset] = Struct;
		reinterpret_cast<T*>(this->data + offset)[0] = value;
	}

	template<class T>
	T getStructure(unsigned int offset) {
		return *reinterpret_cast<T*>(this->data + offset);
	}

public:
	Register(unsigned int sizeInBytes);
	Register();
	virtual ~Register();

	void setSize(unsigned int sizeInBytes);

	int getInteger() {
		return getInteger(0);
	}
	void setInteger(int value) {
		this->setSize(sizeof(int));
		setInteger(0, value);
	}
	std::string getString() {
		return getString(0);
	}
	void setString(const std::string& value) {
		this->setSize(value.length() + sizeof(unsigned int));
		setString(0, value);
	}
	DataType getDataType();

	std::string toString();

	bool operator<(Register* other);
	bool operator==(Register* other);
	int getHash();

	template <class T>
	void setStructure(T value){
		offsetInReg[0] = Struct;
		reinterpret_cast<T*>(this->data)[0] = value;
	}
	template <class T>
	T getStructure(){
		return *reinterpret_cast<T*>(this->data);
	}

};

} /* namespace operators */
#endif /* REGISTER_HPP_ */
