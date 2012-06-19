#ifndef BPLUSTREESEGMENT_HPP_
#define BPLUSTREESEGMENT_HPP_

#include "../buffer/BufferFrame.hpp"
#include "../buffer/BufferManager.hpp"
#include "../buffer/SPSegment.hpp"
#include <assert.h>

namespace std {

class BPlusTreeSegment : public SPSegment {
private:
	// this function returns the tree header and is only allowed to be called on pageID=0, slotID=0
	//SBPTreeHeader* getTreeHeader(BufferFrame& bf);
public:
	BPlusTreeSegment(SIEntry siEntry, BufferManager& bm);
	virtual ~BPlusTreeSegment();


	// TODO
	static TID getTreeHeaderTID(){ return { 0, 0 }; }

	unique_ptr<SBPTreeHeader> getTreeHeader() {
		return this->lookup<SBPTreeHeader>(BPlusTreeSegment::getTreeHeaderTID());
	}

	template <class K>
	SNodeReference createInnerNode(SNodeReference parent, SNodeReference rightNeighbour){
		// reserves page memory for a inner node

		assert(!this->isLeaf(parent));	// parent node type must be inner node

		// init leaf node header
		SNodeHeader header = {
				parent, 0, false, rightNeighbour
		};
		SInnerNode<K> newInner = { header };
		return this->insert<SInnerNode<K>>(newInner);	// SNodeReference equals TID thus we can just return the TID
	}

	template <class K>
	SNodeReference createLeafNode(SNodeReference parent, SNodeReference rightNeighbour){
		// reserves page memory for a leaf node and returns the reference to the new node

		assert(!this->isLeaf(parent));	// parent node type must be inner node

		// init leaf node header
		SNodeHeader header = {
				parent, 0, true, rightNeighbour
		};
		SLeafNode<K> newLeaf = { header };
		return this->insert<SLeafNode<K>>(newLeaf);	// SNodeReference equals TID thus we can just return the TID
	}

	bool setRootNode(SNodeReference newRoot){
		unique_ptr<SBPTreeHeader> header = this->lookup<SBPTreeHeader>(this->getTreeHeaderTID());
		header->rootNode = newRoot;
		return this->update<SBPTreeHeader>(this->getTreeHeaderTID(), move(header));
	}

	//lockNode
	//updateNode
	bool isLeaf(SNodeReference ref);

//	template <class K>
//	bool isKeyLeaf(SNodeReference ref){
//		if(this->isLeaf(ref))
//			return false;
//		else{
//			unique_ptr<InnerNode<K>> node = this->getInnerNode<K>(ref);
//			SInnerNodeElement<K>* firstElem = node->getElementByIndex(0);
//			SNodeReference firstChild = firstElem->leftChild;
//			if(this->isLeaf(firstChild))
//				return true;
//			return false;
//		}
//	}

	template <class K> // key type
	unique_ptr<InnerNode<K>> getInnerNode(SNodeReference nodeRef){
		if(this->isLeaf(nodeRef)){
			cerr << "this is not an inner node" << endl;
			return NULL;
		} else {
			unique_ptr<SInnerNode<K>> nodeData = this->lookup<SInnerNode<K>>(nodeRef);
			unique_ptr<InnerNode<K>> result (new InnerNode<K>(*nodeData, nodeRef));
			return result;
		}
	};

	template <class K> // key type
	unique_ptr<LeafNode<K>> getLeafNode(SNodeReference nodeRef){
		if(!this->isLeaf(nodeRef)){
			cerr << "this is not a leaf node" << endl;
			return NULL;
		} else {
			unique_ptr<SLeafNode<K>> nodeData = this->lookup<SLeafNode<K>>(nodeRef);
			unique_ptr<LeafNode<K>> result (new LeafNode<K>(*nodeData, nodeRef));
			return result;
		}
	};

	template <class K> // key type
	unique_ptr<InnerNode<K>> saveNode(unique_ptr<InnerNode<K>> node){
		// writes data of an InnerNode back to disk
		this->update<SInnerNode<K>>(node->getMyID(), node->getData());
		return node;
	}

	template <class K> // key type
	unique_ptr<LeafNode<K>> saveNode(unique_ptr<LeafNode<K>> node){
		// writes data of LeafNode back to disk
		this->update<SLeafNode<K>>(node->getMyID(), node->getData());
		return node;
	}

	bool deleteNode(SNodeReference ref){
		// deletes any type of node by the node reference
		return this->del(ref);
	}

	template <class K>
	void setParent(SNodeReference ref, SNodeReference par){
		if(this->isLeaf(ref)){
			unique_ptr< LeafNode<K> > leafNode = this->getLeafNode<K>(ref);
			leafNode->setParent(par);
			this->saveNode(move(leafNode));
		} else {
			unique_ptr< InnerNode<K> > innerNode = this->getInnerNode<K>(ref);
			innerNode->setParent(par);
			this->saveNode(move(innerNode));
		}
	}

	// initializes the tree header on pageID=0, slotID=0
	// segment pages are being initialized in initializeSegmentPages (inherited from SPSegment and executed in constructor)
	template <class T>
	void initializeTreeHeader(){
		// additional work to do in BPlusTreeSegment, set first slot of page 0 to contain the tree header
		TID treeHeader = this->insert<SBPTreeHeader>( { {0,0} } ); // init with any root as TID

		// create the root
		SNodeReference root = this->createLeafNode<T>({},{});
		// update the tree header
		unique_ptr<SBPTreeHeader> header = this->lookup<SBPTreeHeader>(treeHeader);
		header->rootNode = root;
		this->update<SBPTreeHeader>(treeHeader, move(header));
	}
};

} /* namespace std */
#endif /* BPLUSTREESEGMENT_HPP_ */
