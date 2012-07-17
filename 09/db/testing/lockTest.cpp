#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <pthread.h>
#include <time.h>
#include <set>

#include "../parser/Parser.hpp"
#include "../parser/Schema.hpp"
#include "../PlanReader/Plan.hpp"
#include "../buffer/BufferManager.h"
#include "../segments/SegmentManager.h"
#include "../schema/SchemaSegment.h"
#include "../lock/LockManager.h"

using namespace std;
using plan::Plan;

BufferManager* bm;
LockManager* lm;

const string fileCreate = "testing/Ex7-2_createTables.sql";
const string filePlan	= "testing/Ex7-2_sample.plan";
const string fileDb		= "db";
const unsigned pagesInRAM	= 100;
const unsigned pagesOnDisk	= 200;
const unsigned threadCount	= 10;
const unsigned maxRand = 100;
//set<TID> done;

static void* lock(void *arg) {
	uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);
	TID tid = {(short int)(rand()%maxRand),rand()%maxRand};
//	TID tid;
//	do {
//		tid = {(short int)(rand()%maxRand),rand()%maxRand};
//	} while (done.count(tid) > 0);
	cout << "-- Thread " << threadNum << " wants to lock " << tid.slotID << tid.pageID << endl;
	uintptr_t r = lm->lock(tid, threadNum);
//	done.insert(tid);
	return reinterpret_cast<void*>(r);
}

static void* unlock(void *arg) {
	uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);
	lm->unlockAll(threadNum);
	return NULL;
}

int main(int argc, char** argv) {
	/*
	// parse schema
	cout << "-- parse schema from " << fileCreate << endl;
	Parser parser(fileCreate);
	unique_ptr<Schema> schema;
	try {
		schema = parser.parse();
		cout << schema->toString() << endl;
	} catch (ParserError& e) {
		cerr << e.what() << endl;
		return -1;
	}


	// setting everything up
	cout << endl << "-- init Buffer- and SegmentManager" << endl;
	BufferManager bm(fileDb, pagesInRAM);
	SegmentManager sm(bm);
	sm.reinitializeSegmentInventory();
	SegmentID spId = sm.create<SchemaSegment>(40);
	SchemaSegment& scs = *sm.get<SchemaSegment>(spId);
	// read schema
	scs.readSchemaFromFile(fileCreate, &sm);
	*/


//	unsigned* threadSeed = new unsigned[threadCount];
//	for (unsigned i=0; i<threadCount; i++)
//	  threadSeed[i] = i*97134;

	lm = new LockManager();
//	bm = new BufferManager(fileDb, pagesInRAM);

	// benchmark
	int clo = clock();

	srand ( time(NULL) );
	pthread_t threads[threadCount];
	pthread_attr_t pattr;
	pthread_attr_init(&pattr);

//	// set all counters to 0
//	for (unsigned i=0; i<pagesOnDisk; i++) {
//	  BufferFrame bf = bm->fixPage(i, true);
//	  reinterpret_cast<unsigned*>(bf.getData())[0]=0;
//	  bm->unfixPage(bf, true);
//	}

	// strict 2PL
	// start lock threads
	for (unsigned i=0; i<threadCount; i++)
	  pthread_create(&threads[i], &pattr, lock, reinterpret_cast<void*>(i));

	// wait for lock threads
	unsigned totalCount = 0;
	for (unsigned i=0; i<threadCount; i++) {
	  void *ret;
	  pthread_join(threads[i], &ret);
	  totalCount+=reinterpret_cast<uintptr_t>(ret);
	}

	cout << "--> done with locking, will continue with unlocking after pause..." << endl;
	sleep(1); // now we have all locks and can do stuff

	// start unlock threads
	for (unsigned i=0; i<threadCount; i++)
	  pthread_create(&threads[i], &pattr, unlock, reinterpret_cast<void*>(i));

	// wait for unlock threads
	for (unsigned i=0; i<threadCount; i++) {
	  pthread_join(threads[i], NULL);
	}


	// benchmark
	cout << "time: " << (clock() - clo) << endl;

//	delete bm;
	delete lm;

	return 0;
}
