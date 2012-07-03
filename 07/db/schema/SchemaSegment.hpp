#ifndef SCHEMASEGMENT_HPP_
#define SCHEMASEGMENT_HPP_

#include "../buffer/SISegment.hpp"
#include "../buffer/SegmentManager.hpp"
#include "../parser/Types.hpp"
using namespace std;

#define LENGTH 100

struct SSchema {
	TID firstRelation;
	TID lastRelation;
};

struct SRelation {
	char name[LENGTH];
	SegmentID segment;
	TID firstAttribute;
	TID lastAttribute;
	TID nextRelation;
};

struct SAttribute {
	char name[LENGTH];
	unsigned int offset;
	Types::Tag type;
	unsigned int lenth;
	bool notNull;
	TID nextAttribute;
};

class SchemaSegment: public SPSegment {
private:
	TID getRelation(string name);
	TID firstRelation();
	TID lastRelation();
	TID getAttribute(string relationName, string attributeName);
	bool isEmpty();

public:
	SchemaSegment(SISegment::SegmentEntry entry, BufferManager& bm);
	virtual ~SchemaSegment();

	SegmentID getRelationSegmentID(string name);
	TID addRelation(string name, SegmentID segment);

	TID addAttribute(string relationName, string attributeName,
			unsigned int tupleOffset, Types::Tag type, unsigned int length,
			bool notNull);
	Types::Tag getAttributeType(string relationName, string attributeName);
	unsigned int getAttributeOffset(string relationName, string attributeName);

	void readSchemaFromFile(string filename, SegmentManager* sm);

};

#endif /* SCHEMASEGMENT_HPP_ */
