/*
 * datastructures.h
 *
 *  Created on: Jun 8, 2012
 *      Author: user
 */

#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_

#include "../parser/Types.hpp"
#include "../segments/datastructures.h"

#define NAME_LENGTH 100

struct SchemaInformation {
	TID firstRelationInformation;
	TID lastRelationInformation;

	// TODO: make stuff more efficient with last-Information entries
};

struct RelationInformation {
	char		name[NAME_LENGTH];
	SegmentID	segment;
	TID			firstAttributeInformation;
	TID			lastAttributeInformation;
	TID			firstIndexInformation;
	TID			nextRelationInformation;		// points to next relation information, if {0,0} this one is last
};

struct AttributeInformation {
	char			name[NAME_LENGTH];
	unsigned int	tupleOffset;
    Types::Tag 		type;
    unsigned int	len;
    bool 			notNull;
    TID				nextAttributeInformation;	// points to next attribute information, if {0,0} this one is last

    // indexing
    bool			isIndexed;
    SegmentID		indexSegmentID;
};

//// currently supports only one-column indexes
//struct IndexInformation {
//	char			attributeName[NAME_LENGTH];	// name of the indexed attribute
//	SegmentID		indexSegment;				// segment that manages the index
//	TID				nextIndexInformation;
//};

#endif /* DATASTRUCTURES_H_ */
