/*
 * BPTree.h
 *
 *  Created on: May 25, 2012
 *      Author: user
 */

#ifndef BPTREE_H_
#define BPTREE_H_

#include "../segments/datastructures.h"
#include "BPlusTreeSegment.h"
#include "../segments/SegmentManager.h"
#include <memory>

namespace std {

// TODO: support comparison function, currently only generic keys are supported
template <class K>
class BPTree {
	unique_ptr<BPlusTreeSegment> seg;
	void insert(K key, TID value, SNodeReference node, SNodeReference* leftC, SNodeReference* rightC){
//		// TODO: assert in the following that when entering a leaf node, the leaf is not full!
		if(node.pageID==0 && node.slotID==0){
			// create new root node
			SNodeReference newRootRef = seg->createInnerNode<K>({0, 0}, {0, 0});
			seg->setRootNode(newRootRef);
			unique_ptr<InnerNode<K> > newRoot = seg->getInnerNode<K>(newRootRef);
			newRoot->addElement({key, *leftC});
			newRoot->setRightChild(0, *rightC);
			seg->saveNode(move(newRoot));

			seg->setParent<K>(*leftC, newRootRef);
			seg->setParent<K>(*rightC, newRootRef);
			return;
		}
		if(seg->isLeaf(node)){
			// entering leaf means it is not full
			unique_ptr< LeafNode<K> > leafNode = seg->getLeafNode<K>(node);
			//assert(!leafNode->isFull());
			leafNode->addElement({ key, value});

			// TODO split
			if(leafNode->willBecomeFull()){
				SNodeReference newLeafNodeRef = seg->createLeafNode<K>(leafNode->getParent(), leafNode->getRightNeighbour());
				leafNode->setRightNeighbour(newLeafNodeRef);

				unique_ptr<LeafNode<K> > newLeafNode = seg->getLeafNode<K>(newLeafNodeRef);	// get new leaf node
				SLeafNodeElement<K>* medianElement = leafNode->getElementByIndex(A-1);		// calc median

				int elements = leafNode->numberOfElements();
				for(int i=A; i<elements; i++){
					SLeafNodeElement<K>* current = leafNode->getElementByIndex(A);		// get current element
					SLeafNodeElement<K> currentTmp = *current;
					newLeafNode->addElement(currentTmp);								// add element to new leaf
					leafNode->deleteElement(currentTmp.key);// remove element from left leaf
					//cout << currentTmp.key << endl;
				}
				newLeafNode = seg->saveNode(move(newLeafNode));											// write new node to disk
				leafNode = seg->saveNode(move(leafNode));
				insert(medianElement->key, (TID){0,0}, leafNode->getParent(), &node, &newLeafNodeRef);	// continue insert with parent
				return;
			}
			seg->saveNode(move(leafNode));													// write modified node to disk

		} else if (leftC != NULL){
			unique_ptr<InnerNode<K>> iNode = seg->getInnerNode<K>(node);
			iNode->addElement({ key, *leftC});
			//int elem = iNode->findMaximumLowerElementIndex(key);
			//iNode->setRightChild(elem, *rightC);
			unsigned int elem = iNode->findMinimumHigherElementIndex(key);
			iNode->setLeftChild(elem, *rightC);
			iNode = seg->saveNode(move(iNode));

			// TODO split
			if(iNode->willBecomeFull()){
				SNodeReference newNodeRef = seg->createInnerNode<K>(iNode->getParent(), iNode->getRightNeighbour());
				iNode->setRightNeighbour(newNodeRef);

				unique_ptr<InnerNode<K> > newNode = seg->getInnerNode<K>(newNodeRef);	// get new leaf node

				SInnerNodeElement<K>* medianElement = iNode->getElementByIndex(A-1);	// calc median
				K medianElementKey = medianElement->key;

				SNodeReference rightmostChild = iNode->getRightChild(iNode->numberOfElements()-1);	// cache rightmost child

				int elements = iNode->numberOfElements();
				for(int i=A; i<elements; i++){
					SInnerNodeElement<K>* current = iNode->getElementByIndex(A);		// get current element
					SInnerNodeElement<K> currentTmp = *current;							// cache data

					newNode->addElement(currentTmp);									// add element to new leaf
					iNode->deleteElement(currentTmp.key);								// remove element from old leaf

					seg->setParent<K>(currentTmp.leftChild, newNodeRef);				// update parent link of child
				}
				// set right child of right most element in new inner node and update parent link
				newNode->setRightChild(newNode->numberOfElements() - 1, rightmostChild);
				seg->setParent<K>(rightmostChild, newNodeRef);


				// remove the median and insert it in parent
				iNode->deleteElement(medianElementKey);

				seg->saveNode(move(newNode));
				iNode = seg->saveNode(move(iNode));		// write new node to disk

				insert(medianElementKey, (TID){0,0}, iNode->getParent(), &node, &newNodeRef);	// continue insert with parent
				return;
			}
			//seg->saveNode(move(iNode));													// write modified node to disk
		} else {
			//Search for the leaf node where to add the key.
			unique_ptr<InnerNode<K>> iNode = seg->getInnerNode<K>(node);
			unsigned int i = iNode->findMinimumHigherElementIndex(key);
			this->insert(key, value, iNode->getLeftChild(i), leftC, rightC);
		}
	}

	TID lookup(K key, SNodeReference node){
		if(seg->isLeaf(node)){
			unique_ptr<LeafNode<K>> leafNode = seg->getLeafNode<K>(node);
			SLeafNodeElement<K>* result = leafNode->getElement(key);
			if(result != NULL){
				return result->value;
			} else {
				// TODO: implemented to search in right neighbour if not found (for concurrency handling)
				cerr << "element not found!" << endl;
				return {};
			}
		}
		else {
			unique_ptr<InnerNode<K>> innerNode = seg->getInnerNode<K>(node);
			unsigned int nextHigher = innerNode->findMinimumHigherElementIndex(key);
			return lookup(key, innerNode->getLeftChild(nextHigher));
		}
	}

void printTree(SNodeReference node) {
	if(node.pageID == 0 && node.slotID == 0){
		return;
	}
	else if(seg->isLeaf(node)) {
		unique_ptr<LeafNode<K> > leafNode = seg->getLeafNode<K>(node);
		cout << "printTree: leafNode: " << node.pageID << ", " << node.slotID
				<< ", with parent: " << leafNode->getParent().pageID << ", " << leafNode->getParent().slotID << endl;
		leafNode->printNode();
	} else {
		//cout << "printTree: innerNode: " << node.pageID << ", " << node.slotID << endl;
		unique_ptr<InnerNode<K> > iNode = seg->getInnerNode<K>(node);
		iNode->printNode();
		for(unsigned int i = 0; i <= iNode->numberOfElements(); i++) {
			SNodeReference ref = iNode->getLeftChild(i);
			//cout << "index: " << i << ", node: " << ref.pageID << ", " << ref.slotID << endl;
			printTree(ref);
			if(i != iNode->numberOfElements())
				cout << iNode->getElementByIndex(i)->key << endl;
			//ref = iNode->getRightChild(i);
			//printTree(ref);
		}
	}
}

public:
	BPTree(SegmentManager* sm) {
		SegmentID sid = sm->create<BPlusTreeSegment>(20);
		seg = sm->get<BPlusTreeSegment>(sid);
		seg->initializeTreeHeader<K>();
	};
	virtual ~BPTree(){};

	void insert(K key, TID value){
		this->insert(key, value, seg->getTreeHeader()->rootNode, NULL, NULL);
	}
	void del(K key){
		// TODO
	}
	TID lookup(K key){
		return this->lookup(key, seg->getTreeHeader()->rootNode);
	}

	void printTree(){
		unique_ptr<InnerNode<K > > node = seg->getInnerNode<K>(seg->getTreeHeader()->rootNode);
		//node->printNode();
		this->printTree(seg->getTreeHeader()->rootNode);
	}

};

} /* namespace std */
#endif /* BPTREE_H_ */
