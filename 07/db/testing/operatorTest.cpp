#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>
#include <unordered_map>

#include "../parser/Parser.hpp"
#include "../parser/Schema.hpp"
#include "../PlanReader/Plan.hpp"
#include "../schema/SchemaSegment.hpp"
//#include "../buffer/BufferManager.hpp"
//#include "../buffer/SlottedPage.hpp"
//#include "../buffer/Segment.hpp"
//#include "../buffer/SPSegment.hpp"
//#include "../buffer/SISegment.hpp"
//#include "../buffer/Record.hpp"
#include "../operators/Register.hpp"
#include "../operators/Operator.hpp"
#include "../operators/TableScan.hpp"
#include "../operators/Print.hpp"
#include "../operators/Sort.hpp"
#include "../operators/MergeJoin.hpp"
#include "../operators/Select.hpp"
#include "../operators/Project.hpp"

using namespace std;
using plan::Plan;
using operators::Register;
using operators::TableScan;

const string fileCreate = "testing/Ex7-2_createTables.sql";
const string filePlan	= "testing/Ex7-2_sample.plan";

operators::Operator* walkTree(const plan::Operator& op, SchemaSegment& scs, SegmentManager& sm){
	plan::OperatorType type = op.getOperatorType();
	operators::Operator* r;
	if(type == plan::OperatorType::Sort){
		const plan::Sort& p = dynamic_cast<const plan::Sort&>(op);
		operators::Sort rr(walkTree(*p.child, scs, sm), p.attributeIds, p.order);
		r = &rr;
	}else if(type == plan::OperatorType::MergeJoin){
		const plan::MergeJoin& p = dynamic_cast<const plan::MergeJoin&>(op);
		operators::MergeJoin rr(walkTree(*p.left, scs, sm), walkTree(*p.right, scs, sm), p.attributeIdsLeft, p.attributeIdsRight, p.cmp);
		r = &rr;
	}else if(type == plan::OperatorType::Select){
		const plan::Select& p = dynamic_cast<const plan::Select&>(op);
		operators::Select rr(walkTree(*p.child, scs, sm), p.attributeIds, p.constants, p.constants);
		r = &rr;
	}else if(type == plan::OperatorType::Project){
		const plan::Project& p = dynamic_cast<const plan::Project&>(op);
		operators::Project rr(walkTree(*p.child, scs, sm), p.attributeIds);
		r = &rr;
	}else if(type == plan::OperatorType::TableScan){
		const plan::TableScan& p = dynamic_cast<const plan::TableScan&>(op);
		TableScan<Register> rr((shared_ptr<SchemaSegment>)&scs, &sm, p.name);
		r = &rr;
	}
	return r;
}

int main(int argc, char** argv) {
//	if (argc != 2) {
//		cerr << "usage: " << argv[0] << " <planFile>" << endl;
//		return -1;
//	}

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
	BufferManager bm("/tmp/db", 10ul * 1024ul * 1024ul); // bogus arguments
	SegmentManager sm(bm);
	SegmentID spId = sm.createSegment(Segment::SegmentType::Schema, 200);
	SchemaSegment& scs = static_cast<SchemaSegment&>(sm.getSegment(spId));
	// read schema
	scs.readSchemaFromFile(fileCreate, &sm);


	// insert tuples
	cout << endl << "-- insert 5 tuples in each relation" << endl;
	SPSegment& students = static_cast<SPSegment&>(sm.getSegment(scs.getRelationSegmentID("student")));
	struct {int id; char name[64];} sstudent;
	for(int i=1; i<=5; i++){
		sstudent.id = i;
//		sstudent.name = "Student "+string(i);
		stringstream ss;
		ss << "Student " << i;
		strcpy(sstudent.name, ss.str().c_str());
		Record r(sizeof(sstudent), (char*) ((void*) (&sstudent)));
		students.insert(r); // TID id =
	}
	SPSegment& lectures = static_cast<SPSegment&>(sm.getSegment(
			scs.getRelationSegmentID("lecture")));
	struct {
		int id;
		char name[64];
	} slecture;
	for (int i = 1; i <= 5; i++) {
		slecture.id = i;
		stringstream ss;
		ss << "Lecture " << i;
		strcpy(slecture.name, ss.str().c_str());
		Record r(sizeof(slecture), (char*) ((void*) (&slecture)));
		lectures.insert(r);
	}
	SPSegment& exams = static_cast<SPSegment&>(sm.getSegment(
			scs.getRelationSegmentID("exam")));
	struct {
		int l_id;
		int s_id;
		int grade;
	} sexam;
	for (int i = 1; i <= 5; i++) {
		sexam.l_id = i;
		sexam.s_id = i; // 5-i
		sexam.grade = i + 3;
		Record r(sizeof(sexam), (char*) ((void*) (&sexam)));
		exams.insert(r);
	}


	// parse query plan
	cout << endl << "-- parse query plan from " << filePlan << endl;
	Plan p;
	p.fromFile(argv[1]);
	p.print(cout);
//	const plan::Operator* root = &p.getRoot();


	// walk down tree and execute the corresponding physical operators
	cout << endl << "-- generate tree of physical operators" << endl;
	operators::Operator* op = walkTree(p.getRoot(), scs, sm);


	// print results
	cout << endl << "-- print results" << endl;
	operators::ScanFile print(op, cout);
	print.open();
	while(print.next()){}
	print.close();

	return 0;
}
