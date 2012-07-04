/*
 * LeafNode.h
 *
 *  Created on: May 25, 2012
 *      Author: user
 */

#ifndef LEAFNODE_H_
#define LEAFNODE_H_

#include "datastructures.h"
#include <iostream>

namespace std {

template <class K>
class LeafNode {
private:
	SNodeReference myReference;
	SLeafNode<K> data;
	SNodeHeader& getHeader(){ return this->data.header;	}
public:
	// TODO: instantiate with comparator function
	LeafNode(SLeafNode<K> data, SNodeReference myReference){
		this->data = data;
		this->myReference = myReference;
	}
	virtual ~LeafNode(){};

	void printNode(){
		cout << "printing leaf node (" << myReference.pageID << ", " << myReference.slotID << "), number of elements: " <<
				getHeader().numberOfElements << " => ";
		for(unsigned int i=0; i<getHeader().numberOfElements; i++){
			cout << "(" << getData().elements[i].key << ", "
					<< getData().elements[i].value.pageID << "-" << getData().elements[i].value.slotID << ")";
		}
		cout << endl;
	}

	unsigned int numberOfElements(){
		return getHeader().numberOfElements;
	}

	bool isLeaf(){ return true; };			// node is a leaf node

	bool isFull(){
		// maximum number of elements reached
		if(getHeader().numberOfElements >= NODE_SIZE){
			return true;
		}
		return false;
	}

	// becomes full on next insert
	bool willBecomeFull(){
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

	SLeafNode<K>& getData(){
		return data;
	}

	SNodeReference getMyID(){
		return myReference;
	}

	// returns -1 if it's the smallest
	int findMaximumLowerElementIndex(K key){
		// returns the node element index which is the next lower one to the given key
		int result = -1;
		for(unsigned int i=0; i<getHeader().numberOfElements; i++){
			if(key >= getData().elements[i].key)
				result = i;
		}
		return result;
	}

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
//		assert(nextLower >= 0 && nextLower < numberOfElements());			// validate range
//		if(key != getData().elements[nextLower].key)
//			cout << getData().elements[nextLower].key << " " << key << endl;
//		assert(getData().elements[nextLower].key == key);					// validate key
//
//		// TODO: make this more efficient, use a list for managing the elements!
//		// delete element
//		for(unsigned int i=nextLower; i < getHeader().numberOfElements-1; i++){
//			getData().elements[i] = getData().elements[i+1];	// overwrite current with next
//		}
//
//		getHeader().numberOfElements -= 1;		// decrement element counter
//		return true;
	}

	bool addElement(SLeafNodeElement<K> element){
		assert(!this->isFull());

		int nextHigher = this->findMinimumHigherElementIndex(element.key);
		SLeafNodeElement<K> tmpLast = element;
		SLeafNodeElement<K> tmpCurrent;
		getHeader().numberOfElements++;
		for(unsigned int i=nextHigher; i <= getHeader().numberOfElements; i++){// we also have to move the very last element (right child)
			tmpCurrent = getData().elements[i];	// cache current element
			getData().elements[i] = tmpLast;	// overwrite with last element
			tmpLast = tmpCurrent;				// save current as last
		}
		return true;

//		//assert(!this->isFull());
//		//if(this->isFull()) return false;
//
//		int nextLower = this->findMaximumLowerElementIndex(element.key);
//
//		// TODO: make this more efficient, use a list for managing the elements!
//		// insert element
//		SLeafNodeElement<K> tmpLast = element;
//		SLeafNodeElement<K> tmpCurrent;
//		getHeader().numberOfElements++;			// increment element counter
//		for(unsigned int i=nextLower+1; i<getHeader().numberOfElements; i++){
//			tmpCurrent = getData().elements[i];	// cache current element
//			getData().elements[i] = tmpLast;	// overwrite with last element
//			tmpLast = tmpCurrent;				// save current as last
//		}
//
//		return true;
	}

	SLeafNodeElement<K>* getElement(K key){
		// returns SInnerNodeElement, given the key of generic type U
		// TODO
		int nextLower = this->findMaximumLowerElementIndex(key);
		if(nextLower > -1 && getData().elements[nextLower].key == key){
			return &(getData().elements[nextLower]);
		} else {
			return 0;
		}
	}

	SLeafNodeElement<K>* getElementByIndex(int i){
		return getData().elements + i;
	}

};

} /* namespace std */
#endif /* LEAFNODE_H_ */
