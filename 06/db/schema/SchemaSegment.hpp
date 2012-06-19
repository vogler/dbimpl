#ifndef SCHEMASEGMENT_HPP_
#define SCHEMASEGMENT_HPP_

#include "../buffer/SISegment.hpp"
#include "../buffer/SegmentManager.hpp"

namespace std {

class SchemaSegment : public SPSegment {
private:
	TID schemaInformation();		// returns schema information TID, always {0,0}
	TID findRelation(string name);	// return {0,0} if not found
	TID firstRelation();			// returns TID of first relation, {0,0} if empty
	TID lastRelation();				// returns TID of last relation, {0,0} if empty
	bool isEmpty();					// returns false if there are one or more relations in the schema

	TID findAttribute(string relationName, string attributeName);
public:
	SchemaSegment(SISegment::SegmentEntry siEntry, BufferManager& bm);
	virtual ~SchemaSegment();

	// initializes the schema header information
	// segment pages are being initialized in initializeSegmentPages (inherited from SPSegment and executed in constructor)
	void initializeSchema();


	// relations
	SegmentID getRelationSegmentID(string relationName);
	TID addRelation(string name, SegmentID segment);

	// attributes
	TID addAttribute(string relationName, string attributeName,
			unsigned int tupleOffset, Types::Tag type, unsigned int length, bool notNull);
    Types::Tag getAttributeType(string relationName, string attributeName);
    unsigned int getAttributeOffset(string relationName, string attributeName);

	// indexes, currently supporting only one column indexes
    void addIndex(string relationName, string attributeName, SegmentID segment);
	bool isIndexed(string relationName, string attributeName);
	SegmentID getIndexSegmentID(string relationName, string attributeName);

	// read and save schema information from file
	void readSchemaFromFile(string filename, SegmentManager* sm);

};

} /* namespace std */
#endif /* SCHEMASEGMENT_HPP_ */
