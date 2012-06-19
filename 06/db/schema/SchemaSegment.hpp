#ifndef SCHEMASEGMENT_HPP_
#define SCHEMASEGMENT_HPP_

#include "../buffer/SISegment.hpp"
#include "../buffer/SegmentManager.hpp"
using namespace std;

#define LENGTH 100

struct SchemaInformation {
	TID firstRelationInformation;
	TID lastRelationInformation;
};

struct RelationInformation {
	char		name[LENGTH];
	SegmentID	segment;
	TID			firstAttributeInformation;
	TID			lastAttributeInformation;
	TID			firstIndexInformation;
	TID			nextRelationInformation;
};

struct AttributeInformation {
	char			name[LENGTH];
	unsigned int	tupleOffset;
    Types::Tag 		type;
    unsigned int	len;
    bool 			notNull;
    TID				nextAttributeInformation;
};


class SchemaSegment : public SPSegment {
private:
	TID schema();
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
			unsigned int tupleOffset, Types::Tag type, unsigned int length, bool notNull);
    Types::Tag getAttributeType(string relationName, string attributeName);
    unsigned int getAttributeOffset(string relationName, string attributeName);

	void readSchemaFromFile(string filename, SegmentManager* sm);

};

#endif /* SCHEMASEGMENT_HPP_ */
