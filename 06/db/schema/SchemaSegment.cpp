#include "SchemaSegment.hpp"
#include <string.h>
#include <assert.h>
#include "../parser/Parser.hpp"
#include "../btree/BTreeSegment.hpp"
#include "../buffer/SISegment.hpp"
#include "../buffer/SPSegment.hpp"

namespace std {

SchemaSegment::SchemaSegment(SISegment::SegmentEntry entry, BufferManager& bm) : SPSegment(bm, vector<BufferFrame*>(1,entry)) {

}

SchemaSegment::~SchemaSegment() {

}


void SchemaSegment::initializeSchema(){
	this->initializeSegmentPages();
	this->insert<SchemaInformation>( { {0,0}, {0,0} });	// there is now first/last relation entry
}

TID SchemaSegment::addRelation(string name, SegmentID segment){
	assert(name.size() <= NAME_LENGTH);		// validate relation name

	// create and insert relation information
	RelationInformation newRelRec = { "", segment, {0,0},	{0,0}, {0,0} };
	name.copy(newRelRec.name, name.size(), 0);
	TID newRel = this->insert<RelationInformation>( newRelRec );

	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schemaInformation());
	if(this->isEmpty()){
		// also update first relation information
		info->firstRelationInformation = newRel;
	} else {
		// udpate link of last relation information to point to new one
		unique_ptr<RelationInformation> currentLast = this->lookup<RelationInformation>(info->lastRelationInformation);
		currentLast->nextRelationInformation = newRel;
		this->update<RelationInformation>(info->lastRelationInformation, move(currentLast));
	}
	// udpate schema informations link to last information
	info->lastRelationInformation = newRel;
	this->update<SchemaInformation>(schemaInformation(), move(info));
	return newRel;
}

SegmentID SchemaSegment::getRelationSegmentID(string relationName){
	TID relationInfoTID = this->findRelation(relationName);
	assert( ! (relationInfoTID.pageID == schemaInformation().pageID && relationInfoTID.slotID == schemaInformation().slotID));
	unique_ptr<RelationInformation> relationInfo = this->lookup<RelationInformation>(relationInfoTID);
	return relationInfo->segment;
}




TID SchemaSegment::addAttribute(string relationName, string attributeName,
		unsigned int tupleOffset, Types::Tag type, unsigned int length, bool notNull){
	assert(relationName.size() <= NAME_LENGTH && attributeName.size() <= NAME_LENGTH);

	// get relation
	TID relationInfoTID = this->findRelation(relationName);
	assert( ! (relationInfoTID.pageID == schemaInformation().pageID && relationInfoTID.slotID == schemaInformation().slotID));
	unique_ptr<RelationInformation> relationInfo = this->lookup<RelationInformation>(relationInfoTID);

	// create and insert attribute information
	AttributeInformation newAttrRec = { "", tupleOffset, type, length, notNull, {0,0}, false, 0 };
	attributeName.copy(newAttrRec.name, attributeName.size(), 0);
	TID newAttr = this->insert<AttributeInformation>( newAttrRec );

	if(relationInfo->firstAttributeInformation.pageID == 0 && relationInfo->firstAttributeInformation.slotID == 0){
		// also update first relation information if there is none yet
		relationInfo->firstAttributeInformation = newAttr;
	} else {
		// udpate link of last attribute information to point to new one
		unique_ptr<AttributeInformation> currentLast = this->lookup<AttributeInformation>(relationInfo->lastAttributeInformation);
		currentLast->nextAttributeInformation = newAttr;
		this->update<AttributeInformation>(relationInfo->lastAttributeInformation, move(currentLast));
	}
	// udpate relation information's link to last attribute information
	relationInfo->lastAttributeInformation = newAttr;
	this->update<RelationInformation>(relationInfoTID, move(relationInfo));
	return newAttr;
}

Types::Tag SchemaSegment::getAttributeType(string relationName, string attributeName){
	TID attrTID = this->findAttribute(relationName, attributeName);
	assert( ! (attrTID.pageID == schemaInformation().pageID && attrTID.slotID == schemaInformation().slotID));
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	return attr->type;
}

unsigned int SchemaSegment::getAttributeOffset(string relationName, string attributeName){
	TID attrTID = this->findAttribute(relationName, attributeName);
	assert( ! (attrTID.pageID == schemaInformation().pageID && attrTID.slotID == schemaInformation().slotID));
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	return attr->tupleOffset;
}



void SchemaSegment::addIndex(string relationName, string attributeName, SegmentID segment){
	TID attrTID = this->findAttribute(relationName, attributeName);
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	assert(attr->isIndexed == false);
	attr->isIndexed = true;
	attr->indexSegmentID = segment;
	this->update(attrTID, move(attr));
}

bool SchemaSegment::isIndexed(string relationName, string attributeName){
	TID attrTID = this->findAttribute(relationName, attributeName);
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	return attr->isIndexed;
}

SegmentID SchemaSegment::getIndexSegmentID(string relationName, string attributeName){
	TID attrTID = this->findAttribute(relationName, attributeName);
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	return attr->indexSegmentID;
}


void SchemaSegment::readSchemaFromFile(string filename, SegmentManager* sm){
	// metadata/schema segment
	Parser p(filename);
	try {
		std::unique_ptr<Schema> schema = p.parse();
		//std::cout << schema->toString() << std::endl;

		for(const Schema::Relation& rel : schema->relations){
			// create segment and save relation information in schema segment
			SegmentID sid = sm->create<SPSegment>(20);
			this->addRelation(rel.name, sid);

			// save attribute information
			unsigned int currentTupleOffset = 0;
			for (const auto& attr : rel.attributes) {
				this->addAttribute(rel.name, attr.name, currentTupleOffset, attr.type, attr.len, attr.notNull);
				if(attr.len != (unsigned int)-1)
					currentTupleOffset += attr.len;
				else
					currentTupleOffset += 4;
			}

			// create indexes for primary key attributes
			for (unsigned keyId : rel.primaryKey){
				SegmentID indexSID = sm->create<BPlusTreeSegment>(5);
				this->addIndex(rel.name, rel.attributes[keyId].name, indexSID);
			}
		}
	} catch (ParserError& e) {
		std::cerr << e.what() << std::endl;
	}
}


// -----------------------------------------------
// private methods
// -----------------------------------------------
TID SchemaSegment::schemaInformation(){
	return {0,0};
}

TID SchemaSegment::findRelation(string name){
	assert(name.size() <= NAME_LENGTH);
	TID relationTID = this->firstRelation();
	while( ! (relationTID.pageID == schemaInformation().pageID && relationTID.slotID == schemaInformation().slotID) ){
		unique_ptr<RelationInformation> info = this->lookup<RelationInformation>(relationTID);
		if(strcmp(info->name, name.c_str()) == 0){
			// we found a relation with the given name
			//cout << "comparison successful: " << info->name << ", " << name.c_str() << endl;
			return relationTID;
		}
		relationTID = info->nextRelationInformation;
	}
	return relationTID;
}

TID SchemaSegment::firstRelation(){
	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schemaInformation());
	TID result = info->firstRelationInformation;
	return result;
}

TID SchemaSegment::lastRelation(){
	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schemaInformation());
	TID result = info->lastRelationInformation;
	return result;
}

bool SchemaSegment::isEmpty(){
	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schemaInformation());
	TID last = info->lastRelationInformation;
	if(last.pageID == schemaInformation().pageID && last.slotID == schemaInformation().slotID){
		return true;
	} else {
		return false;
	}
}

TID SchemaSegment::findAttribute(string relationName, string attributeName){
	assert(attributeName.size() <= NAME_LENGTH);

	TID relationTID = this->findRelation(relationName);
	unique_ptr<RelationInformation> relation = this->lookup<RelationInformation>(relationTID);

	TID attributeTID = relation->firstAttributeInformation;
	while( ! (attributeTID.pageID == schemaInformation().pageID && attributeTID.slotID == schemaInformation().slotID) ){
		unique_ptr<AttributeInformation> info = this->lookup<AttributeInformation>(attributeTID);
		if(strcmp(info->name, attributeName.c_str()) == 0){
			// we found an attribute with the given name
			return attributeTID;
		}
		attributeTID = info->nextAttributeInformation;
	}
	return attributeTID;
}

} /* namespace std */
