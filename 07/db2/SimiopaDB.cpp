//============================================================================
// Name        : SimiopaDB.cpp
// Author      : Vlad Popa, Simon Groetzinger
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "buffer/BufferManager.h"
#include "segments/SegmentManager.h"
#include "bplustrees/BPlusTreeSegment.h"
#include "bplustrees/datastructures.h"
#include "buffer/BufferFrame.h"
#include "bplustrees/BPTree.h"
#include "parser/Parser.hpp"
#include "parser/Schema.hpp"
#include "schema/SchemaSegment.h"
#include "operators/Register.hpp"
#include "operators/TableScan.hpp"
#include "operators/Print.hpp"

unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;

int main(int argc, char** argv) {
	//BufferManager *bm = new BufferManager("vlopa", 1);
	//bm->fixPage(1, false);
	printf("I am SimiopaDB!!!\nQué pasa amigo? :P\n\n");

	if (argc==5) {
	  pagesOnDisk = atoi(argv[2]);
	  pagesInRAM = atoi(argv[3]);
	  threadCount = atoi(argv[4]);
	} else {
	  cerr << "usage: " << argv[0] << " <file> <pagesOnDisk> <pagesInRAM> <threads>" << endl;
	  exit(1);
	}

	BufferManager* bm = new BufferManager(argv[1], pagesInRAM);
	SegmentManager* sm = new SegmentManager(*bm);
	sm->reinitializeSegmentInventory();


	/*
	// checking regular segments
	cout << "checking regular segments..." << endl;
	SegmentID sid = sm->create<std::Segment>(3);
	unique_ptr<Segment> s = sm->get<std::Segment>(sid);
	cout << "getSize: " << s->getSize() << endl;
	cout << "getGlobalPageID: " << s->getGlobalPageID(0) << endl;
	// check page iteration
	for(unsigned int i=0; i<s->getSize(); i++){
		cout << "global page id of page with local id=" << i << ": " << s->getGlobalPageID(i) << endl;
	}
	cout << endl;


	// checking slotted page segments
	cout << "checking slotted page segments..." << endl;
	sm->create<std::SPSegment>(2);
	sm->create<std::SPSegment>(5);
	//cout << sm->getNumberOfSegments() << endl;
	unique_ptr<SPSegment> sps = sm->get<std::SPSegment>(sm->create<std::SPSegment>(2));
	TID test = sps->insert<Record>({ 1 });
	cout << test.pageID << ", " << test.slotID << endl;
	test = sps->insert<RecordBig>({ 2, 100 });
	cout << test.pageID << ", " << test.slotID << endl;

	sps = sm->get<std::SPSegment>(sm->create<std::SPSegment>(2));
	test = sps->insert<Record>({ 12345 });
	cout << test.pageID << ", " << test.slotID << endl;

	// lookup and check record
	unique_ptr<Record> r = sps->lookup<Record>(test);
	cout << r->test << endl;

	// modify record
	//Record updated = { 54321 };
	r->test = 54321;
	cout << "update with number 54321, successful? " << sps->update(test, move(r)) << endl;

	// lookup and check again
	cout << "looking up updated record: " << sps->lookup<Record>(test)->test << endl;

	// delete record
	cout << "delete successful? " << sps->del(test) << endl;

	// check if deleted
	cout << "lookup successful? " << (sps->lookup<Record>(test)!=NULL ? true : false) << endl;

	// checking inherited functions
	cout << "getSize: " << sps->getSize() << endl;
	cout << "getGlobalPageID: " << sps->getGlobalPageID(0) << endl;
	//cout << "getGlobalPageID with out of range local id: " << sps->getGlobalPageID(100) << endl;


	// checking b plus trees
	typedef int KeyType;

	// checking tree init
	cout << "checking tree initilization..." << endl;
	sid = sm->create<BPlusTreeSegment>(20);
	unique_ptr<BPlusTreeSegment> ts = sm->get<BPlusTreeSegment>(sid);
	ts->initializeTreeHeader<KeyType>();
	cout << "root node (should return 0,1): " << ts->getTreeHeader()->rootNode.pageID << ", " << ts->getTreeHeader()->rootNode.slotID << endl;

	// checking (root) node access
	cout << "checking (root) node access..." << endl;
	SNodeReference leaf = ts->getTreeHeader()->rootNode;
	unique_ptr<LeafNode<KeyType>> leafNode = ts->getLeafNode<KeyType>(ts->getTreeHeader()->rootNode);
	cout << "try to call getLeafNode with root reference..." << endl;
	cout << "is full? " << leafNode->isFull() << endl;

	// checking node creation and modification
	cout << "checking node insertion and modification..." << endl;
	SNodeReference inner = ts->createInnerNode<KeyType>({}, {});
	ts->setRootNode(inner);
	leafNode->setParent(inner);
	unique_ptr<InnerNode<KeyType>> innerNode = ts->getInnerNode<KeyType>(inner);
	innerNode = ts->saveNode(move(innerNode));
	leafNode = ts->saveNode(move(leafNode));
	if(ts->getTreeHeader()->rootNode.pageID == inner.pageID && ts->getTreeHeader()->rootNode.slotID == inner.slotID)
		cout << "root node is correct" << endl;
	else
		cerr << "root node is wrong!" << endl;


	// checking leaf node operations
	cout << "checking node operations..." << endl;
	cout << "try to get key which is not there..." << endl;
	KeyType keyToFind = 3;
	leafNode = ts->getLeafNode<KeyType>(leaf);
	SLeafNodeElement<KeyType>* nodeElement = leafNode->getElement(keyToFind);
	if(nodeElement!=NULL){
		cout << "element with key '" << keyToFind << "' found. value: " << nodeElement->value.pageID << ", " << nodeElement->value.slotID << endl;
	} else {
		cout << "key not found (correct)" << endl;
	}
	assert(nodeElement==NULL);

	leafNode->printNode();
	cout << "try to add element with key 3..." << endl;
	leafNode->addElement({ 3, leaf});
	leafNode->printNode();
	cout << "try to add element with key 5..." << endl;
	leafNode->addElement({ 5, leaf});
	leafNode->printNode();
	cout << "try to add element with key 1..." << endl;
	leafNode->addElement({ 1, leaf});
	leafNode->printNode();
	cout << "try to add element with key 4..." << endl;
	leafNode->addElement({ 4, leaf});
	leafNode->printNode();
	unsigned int elements = leafNode->numberOfElements();
	cout << endl;

	// check saving to disk
	cout << "check saving to disk..." << endl;
	leafNode = ts->saveNode(move(leafNode));
	// TODO: is this a memory leak???
	leafNode = ts->getLeafNode<KeyType>(leaf);
	cout << "retrieved node after saving to disk: ";
	leafNode->printNode();
	assert(elements == leafNode->numberOfElements());
	cout << endl;

	// check fetching existing element
	cout << "try to get existing element..." << endl;
	leafNode = ts->getLeafNode<KeyType>(leaf);
	nodeElement = leafNode->getElement(keyToFind);
	if(nodeElement!=NULL){
		cout << "element with key '" << keyToFind << "' found. value: " << nodeElement->value.pageID << ", " << nodeElement->value.slotID << " (correct)" << endl;
	} else {
		cout << "key not found" << endl;
	}
	assert(nodeElement!=NULL);
	cout << endl;

	// check delete
	cout << "checking delete..." << endl;
	leafNode->printNode();
	cout << "delete 3...";
	leafNode->deleteElement(keyToFind);
	leafNode->printNode();
	cout << "delete 5...";
	keyToFind = 5;
	leafNode->deleteElement(keyToFind);
	leafNode->printNode();
	cout << "delete 1...";
	keyToFind = 1;
	leafNode->deleteElement(keyToFind);
	leafNode->printNode();
	assert(leafNode->numberOfElements()==1);

	// check saving to disk
	elements = leafNode->numberOfElements();
	cout << "again, check saving to disk..." << endl;
	leafNode = ts->saveNode(move(leafNode));
	// TODO: is this a memory leak???
	leafNode = ts->getLeafNode<KeyType>(leaf);
	cout << "retrieved node after saving to disk: ";
	leafNode->printNode();
	assert(elements == leafNode->numberOfElements());
	cout << endl;

	cout << "LEAF NODE CHECKS SUCCESSFULL :)" << endl << endl;


	// checking inner node operations
	cout << "checking node operations..." << endl;
	cout << "try to get key which is not there..." << endl;
	keyToFind = 3;
	innerNode = ts->getInnerNode<KeyType>(inner);
	SInnerNodeElement<KeyType>* nodeElementInner = innerNode->getElement(keyToFind);
	if(nodeElementInner!=NULL){
		cout << "element with key '" << keyToFind << "' found. leftChild: " << nodeElementInner->leftChild.pageID << ", " << nodeElementInner->leftChild.slotID << endl;
	} else {
		cout << "key not found (correct)" << endl;
	}
	assert(nodeElementInner==NULL);

	innerNode->printNode();
	cout << "try to add element with key 3..." << endl;
	innerNode->addElement({ 3, inner});
	innerNode->printNode();
	cout << "try to add element with key 5..." << endl;
	innerNode->addElement({ 5, inner});
	innerNode->printNode();
	cout << "try to add element with key 1..." << endl;
	innerNode->addElement({ 1, inner});
	innerNode->printNode();
	cout << "try to add element with key 4..." << endl;
	innerNode->addElement({ 4, inner});
	innerNode->printNode();
	elements = innerNode->numberOfElements();
	cout << endl;

	// check saving to disk
	cout << "check saving to disk..." << endl;
	innerNode = ts->saveNode(move(innerNode));
	// TODO: is this a memory leak???
	innerNode = ts->getInnerNode<KeyType>(inner);
	cout << "retrieved node after saving to disk: ";
	innerNode->printNode();
	assert(elements == innerNode->numberOfElements());
	cout << endl;

	// check fetching existing element
	cout << "try to get existing element..." << endl;
	innerNode = ts->getInnerNode<KeyType>(inner);
	nodeElementInner = innerNode->getElement(keyToFind);
	if(nodeElementInner!=NULL){
		cout << "element with key '" << keyToFind << "' found. leftChild: " << nodeElementInner->leftChild.pageID << ", " << nodeElementInner->leftChild.slotID << " (correct)" << endl;
	} else {
		cout << "key not found" << endl;
	}
	assert(nodeElementInner!=NULL);
	cout << endl;

	// check delete
	cout << "checking delete..." << endl;
	innerNode->printNode();
	cout << "delete 3...";
	innerNode->deleteElement(keyToFind);
	innerNode->printNode();
	cout << "delete 5...";
	keyToFind = 5;
	innerNode->deleteElement(keyToFind);
	innerNode->printNode();
	cout << "delete 1...";
	keyToFind = 1;
	innerNode->deleteElement(keyToFind);
	innerNode->printNode();
	assert(innerNode->numberOfElements()==1);

	// check saving to disk
	elements = innerNode->numberOfElements();
	cout << "again, check saving to disk..." << endl;
	innerNode = ts->saveNode(move(innerNode));
	// TODO: is this a memory leak???
	innerNode = ts->getInnerNode<KeyType>(inner);
	cout << "retrieved node after saving to disk: ";
	innerNode->printNode();
	assert(elements == innerNode->numberOfElements());
	cout << endl;

	//cout << "isKeyLeaf check: " << ts->isKeyLeaf<KeyType>(ts->getTreeHeader()->rootNode) << endl;

	cout << "INNER NODE CHECKS SUCCESSFULL :)" << endl << endl;


	// bptree checks
	cout << "B+-Tree checks..." << endl;
	BPTree<KeyType>* tree = new BPTree<KeyType>(sm);
	cout << "simple insert/lookup checks (only operating with root as leaf node)..." << endl;
	tree->insert(1, {1,2});
	tree->insert(5, {5,6});
	tree->insert(3, {3,4});
	cout << "3 inserts successful...";
	assert(tree->lookup(5).slotID==5 && tree->lookup(5).pageID==6);
	assert(tree->lookup(1).slotID==1 && tree->lookup(1).pageID==2);
	assert(tree->lookup(3).slotID==3 && tree->lookup(3).pageID==4);
	cout << "3 lookups successful. check :)" << endl;

	cout << "inserting 1000 elements..." << endl;
	for(int i=1000; i>10; i--){
		tree->insert(i, {(unsigned short int)i,i} );
	}
	cout << "1000 inserts successful, printing tree..." << endl;
	tree->printTree();

	cout << "further lookup checks...";
	assert(tree->lookup(500).slotID==500 && tree->lookup(500).pageID==500);
	assert(tree->lookup(123).slotID==123 && tree->lookup(123).pageID==123);
	cout << "successful!" << endl;
	*/


	cout << "---------- SCHEMA SEGMENT TESTS ----------" << endl;
	SegmentID schemaSegmentSID = sm->create<SchemaSegment>(20);
	unique_ptr<SchemaSegment> schemaSegment = sm->get<SchemaSegment>(schemaSegmentSID);
	schemaSegment->initializeSchema();

	cout << "--- testing relation management..." << endl;
	SegmentID newRelation = sm->create<std::SPSegment>(5);
	cout << "adding relation with name 'users' and segmentID '" << newRelation << "'" << endl;
	schemaSegment->addRelation("users", newRelation);
	SegmentID result = schemaSegment->getRelationSegmentID("users");
	cout << "looking up relation 'users'..." << result << endl;
	assert(newRelation==result);

	newRelation = sm->create<std::SPSegment>(5);
	cout << "adding relation with name 'companies' and segmentID '" << newRelation << "'" << endl;
	schemaSegment->addRelation("companies", newRelation);
	result = schemaSegment->getRelationSegmentID("companies");
	cout << "looking up relation 'companies'..." << result << endl;
	assert(newRelation==result);

	newRelation = sm->create<std::SPSegment>(5);
	cout << "adding relation with name 'cars' and segmentID '" << newRelation << "'" << endl;
	schemaSegment->addRelation("cars", newRelation);
	result = schemaSegment->getRelationSegmentID("cars");
	cout << "looking up relation 'cars'..." << result << endl;
	assert(newRelation==result);
	cout << "add and lookup relation information successful!" << endl << endl;

	cout << "--- testing attribute management...";
	schemaSegment->addAttribute("cars", "numberOfSeats", 0, Types::Tag::Integer, 4, true);
	schemaSegment->addAttribute("cars", "brandName", 4, Types::Tag::Char, 100, false);
	schemaSegment->addAttribute("cars", "numberOfDoors", 104, Types::Tag::Integer, 4, true);
	assert(schemaSegment->getAttributeType("cars", "numberOfSeats")==Types::Tag::Integer);
	assert(schemaSegment->getAttributeType("cars", "brandName")==Types::Tag::Char);
	assert(schemaSegment->getAttributeType("cars", "numberOfDoors")==Types::Tag::Integer);
	assert(schemaSegment->getAttributeOffset("cars", "numberOfSeats")==0);
	assert(schemaSegment->getAttributeOffset("cars", "brandName")==4);
	assert(schemaSegment->getAttributeOffset("cars", "numberOfDoors")==104);
	cout << "successful! :)" << endl;

	cout << "--- testing index management...";
	assert(schemaSegment->isIndexed("cars", "numberOfSeats") == false);
	assert(schemaSegment->isIndexed("cars", "brandName") == false);
	assert(schemaSegment->isIndexed("cars", "numberOfDoors") == false);

	schemaSegment->addIndex("cars", "numberOfSeats", 200);
	schemaSegment->addIndex("cars", "brandName", 201);
	schemaSegment->addIndex("cars", "numberOfDoors", 202);

	assert(schemaSegment->isIndexed("cars", "numberOfSeats") == true);
	assert(schemaSegment->isIndexed("cars", "brandName") == true);
	assert(schemaSegment->isIndexed("cars", "numberOfDoors") == true);
	assert(schemaSegment->getIndexSegmentID("cars", "numberOfSeats") == 200);
	assert(schemaSegment->getIndexSegmentID("cars", "brandName") == 201);
	assert(schemaSegment->getIndexSegmentID("cars", "numberOfDoors") == 202);
	cout << "successful! :)" << endl;

	cout << "--- testing schema loading/saving from file..." << endl;

	cout << "processing schema information from file...";
	schemaSegment->readSchemaFromFile("./parser/test.sql", sm);
	cout << "successful!" << endl;

	cout << "processed the following schema:" << endl;
	Parser p("./parser/test.sql");
	try {
		std::unique_ptr<Schema> schema = p.parse();
		std::cout << schema->toString() << std::endl;

		cout << "testing saved information..." << endl;
		for(const Schema::Relation& rel : schema->relations){
			cout << "segment id of relation '" << rel.name << "': " << schemaSegment->getRelationSegmentID(rel.name) << endl;
			cout << "----------" << endl;
			for (const auto& attr : rel.attributes) {
				assert(schemaSegment->getAttributeType(rel.name, attr.name) == attr.type);
				cout << "attribute '" << attr.name << "' found with offset: "
						<< schemaSegment->getAttributeOffset(rel.name, attr.name) << endl;
			}
			cout << "----------" << endl;
			for (unsigned keyId : rel.primaryKey){
				assert(schemaSegment->isIndexed(rel.name, rel.attributes[keyId].name) == true);
				cout << "attribute '" << rel.attributes[keyId].name << "' is indexed in segment: "
						<< schemaSegment->getIndexSegmentID(rel.name, rel.attributes[keyId].name) << endl;
			}
			cout << endl;
		}
		cout << "successful!" << endl;
	} catch (ParserError& e) {
		std::cerr << e.what() << std::endl;
	}
	cout << "successful! :)" << endl;


	cout << "--- testing register class..." << endl;
//	operators::Register* reg = new operators::Register(1000);
//	reg->setInteger(0, 300);
//	assert(reg->getInteger(0)==300);
//
	string testString = "hallo";
//	reg->setString(4, testString);
//	assert(reg->getString(4)=="hallo");
//
	Record testRecord = { 123 };
//	reg->setStructure<Record>(9,testRecord);
//	assert(reg->getStructure<Record>(9).test == 123);

	operators::Register* reg = new operators::Register();
	operators::Register* reg1 = new operators::Register();
	reg1->setInteger(444);
	reg->setInteger(333);
	assert(reg->getInteger()==333);
	assert(reg->getDataType()==operators::DataType::Integer);
	assert(reg->operator<(reg1));
	assert(! (reg->operator==(reg1)));
	reg1->setInteger(333);
	assert(! (reg->operator<(reg1)));
	assert(reg->operator==(reg1));
	reg1->setInteger(222);;
	assert(! (reg->operator<(reg1)));
	assert(! (reg->operator==(reg1)));


	testString = "hallo123";
	reg->setString(testString);
	assert(reg->getString()=="hallo123");
	assert(reg->getDataType()==operators::DataType::String);
	reg->setStructure<Record>(testRecord);
	assert(reg->getStructure<Record>().test == 123);
	assert(reg->getDataType()==operators::DataType::Struct);

	cout << "successful! :)" << endl << endl;

	cout << "--- testing operators..." << endl;
	cout << "--- testing TableScan..." << endl;
	SegmentID usersSID = schemaSegment->getRelationSegmentID("users");
	unique_ptr<SPSegment> users = sm->get<SPSegment>(usersSID);
//	TID user1 = users->insert<Record>({101});
//	TID user2 = users->insert<Record>({102});
//	TID user3 = users->insert<Record>({103});
	users->insert<Record>({101});
	users->insert<Record>({102});
	users->insert<Record>({103});
	shared_ptr<SchemaSegment> schemaSegmentShared = move(schemaSegment);

	operators::TableScan<Record>* usersTableScan =
		new operators::TableScan<Record>(schemaSegmentShared, sm, "users");
	usersTableScan->open();
	assert(usersTableScan->next() == true);
	assert(usersTableScan->getOutput().back()->getStructure<Record>().test == 101);
	assert(usersTableScan->next() == true);
	assert(usersTableScan->getOutput().back()->getStructure<Record>().test == 102);
	assert(usersTableScan->next() == true);
	assert(usersTableScan->getOutput().back()->getStructure<Record>().test == 103);
	assert(usersTableScan->next() == false);
	usersTableScan->close();

	cout << "--- testing Print..." << endl;
	operators::Print* usersPrint = new operators::Print(usersTableScan, std::cout);
	usersPrint->open();
	while(usersPrint->next());
	usersPrint->close();
	cout << "successful! :)" << endl << endl;


	delete sm;
	delete bm;

	return EXIT_SUCCESS;
}
