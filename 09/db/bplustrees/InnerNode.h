/*
 * InnerNode.h
 *
 *  Created on: May 25, 2012
 *      Author: user
 */

#ifndef INNERNODE_H_
#define INNERNODE_H_

#include "datastructures.h"
#include <iostream>

namespace std {

template <class K>
class InnerNode {
private:
	SNodeReference myReference;
	SInnerNode<K> data;
	SNodeHeader& getHeader(){ return this->data.header;	}
public:
	// TODO: instantiate with comparator function
	//InnerNode(SInnerNode<K> data);//compFunc
	InnerNode(SInnerNode<K> data, SNodeReference myReference){
		this->data = data;
		this->myReference = myReference;
	}
	virtual ~InnerNode() {}

	void printNode(){
		cout << "printing inner node (" << myReference.pageID << ", " << myReference.slotID << "), number of elements: " <<
				getHeader().numberOfElements << " => ";
		for(unsigned int i=0; i<getHeader().numberOfElements; i++){
			cout << "(" << getData().elements[i].key << ", "
					<< getData().elements[i].leftChild.pageID << "-" << getData().elements[i].leftChild.slotID << ")";
		}
		cout << endl;
	}

	unsigned int numberOfElements(){
		//cout << "number of elements: " << getHeader().numberOfElements << endl;
		return getHeader().numberOfElements;
	}

	bool isLeaf(){ return false; };			// node is a leaf node

	bool isFull(){
		// maximum number of elements reached
		SNodeHeader header = data.header;
		if(header.numberOfElements >= NODE_SIZE){ // we need the last slot for the right child
			return true;
		}
		return false;
	}

	// becomes full on next insert
	bool willBecomeFull(){
		// maximum number of elements overflowed
		SNodeHeader header = data.header;
		if(header.numberOfElements >= NODE_SIZE-1){ // we need the last slot for the right child
			return true;
		}
		return false;
	}

	SNodeReference getParent(){
		return data.header.parent;
	}

	void setParent(SNodeReference par){
		data.header.parent = par;
	}

	SNodeReference getRightNeighbour(){
		return data.header.rightNeighbour;
	}

	void setRightNeighbour(SNodeReference n){
		data.header.rightNeighbour = n;
	}

	SInnerNode<K>& getData(){
		return data;
	}

	SNodeReference getMyID(){
		return myReference;
	}

	// returns -1 if it's the smallest
	// TODO: do NOT use it any more
	int findMaximumLowerElementIndex(K key){
		// returns the node element index which is the next lower one to the given key
		int result = -1;
		for(unsigned int i=0; i<getHeader().numberOfElements; i++){
			if(key >= getData().elements[i].key)
				result = i;
		}
		return result;
	}

	// returns 0 if key is lowest element
	// returns numberOfElements if key is the highest one
	unsigned int findMinimumHigherElementIndex(K key){
		for(unsigned int i=0; i<getHeader().numberOfElements; i++){
			if(getData().elements[i].key > key)
				return i;
		}
		return getHeader().numberOfElements;
	}

	// delete element by generic key type
	bool deleteElement(K key){
		unsigned int nextHigher = this->findMinimumHigherElementIndex(key);
		assert(nextHigher > 0 && nextHigher <= numberOfElements());
		assert(getData().elements[nextHigher-1].key == key);
		for(unsigned int i=nextHigher; i < getHeader().numberOfElements; i++){
			getData().elements[i-1] = getData().elements[i];
		}
		getHeader().numberOfElements -= 1;
		return true;

//		unsigned int nextLower = this->findMaximumLowerElementIndex(key);	// find next lower element
//		assert(nextLower >= 0 && nextLower < numberOfElements());	// validate range
//		assert(getData().elements[nextLower].key == key);			// validate key
//		// TODO: make this more efficient, use a list for managing the elements!
//		// delete element
//		for(unsigned int i=nextLower; i < getHeader().numberOfElements; i++){ // we also need to move the very last element (right child)
//			getData().elements[i] = getData().elements[i+1];	// overwrite current with next
//		}
//		getHeader().numberOfElements -= 1;		// decrement element counter
//		return true;
	}

	bool addElement(SInnerNodeElement<K> element){
		assert(!this->isFull());

		int nextHigher = this->findMinimumHigherElementIndex(element.key);
		SInnerNodeElement<K> tmpLast = element;
		SInnerNodeElement<K> tmpCurrent;
		getHeader().numberOfElements++;
		for(unsigned int i=nextHigher; i <= getHeader().numberOfElements; i++){// we also have to move the very last element (right child)
			tmpCurrent = getData().elements[i];	// cache current element
			getData().elements[i] = tmpLast;	// overwrite with last element
			tmpLast = tmpCurrent;				// save current as last
		}
		return true;

//		int nextLower = this->findMaximumLowerElementIndex(element.key);
//		// TODO: make this more efficient, use a list for managing the elements!
//		// insert element
//		SInnerNodeElement<K> tmpLast = element;
//		SInnerNodeElement<K> tmpCurrent;
//		getHeader().numberOfElements++;			// increment element counter
//		for(unsigned int i=nextLower+1; i<=getHeader().numberOfElements; i++){ // we also have to move the very last element (right child)
//			tmpCurrent = getData().elements[i];	// cache current element
//			getData().elements[i] = tmpLast;	// overwrite with last element
//			tmpLast = tmpCurrent;				// save current as last
//		}
//		return true;
	}



	SInnerNodeElement<K>* getElement(K key){
		// returns SInnerNodeElement, given the key of generic type U
		// TODO
		int nextLower = this->findMaximumLowerElementIndex(key);
		if(nextLower > -1 && getData().elements[nextLower].key == key){
			return &(getData().elements[nextLower]);
		} else {
			return 0;
		}
	}

	SInnerNodeElement<K>* getElementByIndex(int i){
		return getData().elements + i;
	}

	SNodeReference getRightChild(unsigned int index){
		assert(index >= 0 && index < numberOfElements());	// validate index
		return getData().elements[index+1].leftChild;
	}
	SNodeReference getLeftChild(unsigned int index){
		// validate index, numberOfElements()+1 is valid because its left child is the right one of the last element
		assert(index >= 0 && index < numberOfElements()+1);
		return getData().elements[index].leftChild;
	}
	void setRightChild(unsigned int index, SNodeReference child){
		assert(index >= 0 && index < numberOfElements());	// validate index
		getData().elements[index+1].leftChild = child;
	}
	void setLeftChild(unsigned int index, SNodeReference child){
		// validate index, numberOfElements()+1 is valid because its left child is the right one of the last element
		assert(index >= 0 && index < numberOfElements()+1);
		getData().elements[index].leftChild = child;
	}
};

} /* namespace std */
#endif /* INNERNODE_H_ */
