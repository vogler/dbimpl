/*
 * datastructures.h
 *
 *  Created on: May 24, 2012
 *      Author: user
 */

#ifndef BPTREES_DATASTRUCTURES_H_
#define BPTREES_DATASTRUCTURES_H_

#include "../segments/datastructures.h"

//#define NODE_SIZE = 100
static const unsigned int A = 5;
static const unsigned int NODE_SIZE = 2*A-1;

//struct SNodeReference {
//	PageID pageID;
//	// TODO: implement offset usage to be able to use a page for more than one node
//	// unsigned int offset;
//};

typedef TID SNodeReference;

struct SBPTreeHeader {
	//SNodeReference firstFreeNode;	// points to first free node (memory after all existing nodes) in the segment
	SNodeReference rootNode;		// points to root node
};

// header of any node
struct SNodeHeader {
	SNodeReference parent;			// reference to parent node
	unsigned int numberOfElements;	// number of elements that are currently in the node
	bool isLeafNode;				// which type of node (inner or leaf)
	SNodeReference rightNeighbour;	// right neighbour of the node, required for lookup when using multiple threads
};

template <class T>
struct SInnerNodeElement {
	T key;						// the actual key
	SNodeReference leftChild;	// reference to left child
};

template <class T>
struct SInnerNode {
	// NODE HEADER MUST BE THE FIRST ELEMENT IN THIS STRUCT
	SNodeHeader header;							// node header with meta information
	SInnerNodeElement<T> elements[NODE_SIZE+2];	// elements (+1 because of last right child)
	// => add NODE_SIZE+1 elements so we have a right child for the last element
};

template <class T>
struct SLeafNodeElement {
	T key;		// the actual key
	TID value;	// the value
};

template <class T>
struct SLeafNode {
	// NODE HEADER MUST BE THE FIRST ELEMENT IN THIS STRUCT
	SNodeHeader header;							// node header with meta information
	SLeafNodeElement<T> elements[NODE_SIZE+1];	// elements
};

#endif /* DATASTRUCTURES_H_ */
