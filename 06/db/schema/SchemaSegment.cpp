#include "SchemaSegment.hpp"
#include <string.h>
#include <assert.h>
#include "../parser/Parser.hpp"
#include "../buffer/SISegment.hpp"
#include "../buffer/SPSegment.hpp"

namespace std {

SchemaSegment::SchemaSegment(SISegment::SegmentEntry entry, BufferManager& bm) : SPSegment(bm, vector<BufferFrame*>(1,entry)) {

}

SchemaSegment::~SchemaSegment() {

}


TID SchemaSegment::addRelation(string name, SegmentID segment){
	assert(name.size() <= LENGTH);

	// create and insert relation information
	RelationInformation newRelRec = { "", segment, 0, 0, 0 };
	name.copy(newRelRec.name, name.size(), 0);
	TID newRel = this->insert<RelationInformation>( newRelRec );

	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schema());
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
	this->update<SchemaInformation>(schema(), move(info));
	return newRel;
}

SegmentID SchemaSegment::getRelationSegmentID(string relationName){
	TID relationInfoTID = this->getRelation(relationName);
	unique_ptr<RelationInformation> relationInfo = this->lookup<RelationInformation>(relationInfoTID);
	return relationInfo->segment;
}




TID SchemaSegment::addAttribute(string relationName, string attributeName,
		unsigned int tupleOffset, Types::Tag type, unsigned int length, bool notNull){
	assert(relationName.size() <= LENGTH && attributeName.size() <= LENGTH);

	// get relation
	TID relationInfoTID = this->getRelation(relationName);
	unique_ptr<RelationInformation> relationInfo = this->lookup<RelationInformation>(relationInfoTID);

	// create and insert attribute information
	AttributeInformation newAttrRec = { "", tupleOffset, type, length, notNull, 0, false, 0 };
	attributeName.copy(newAttrRec.name, attributeName.size(), 0);
	TID newAttr = this->insert<AttributeInformation>( newAttrRec );

	if(SlottedPage::getPageID(relationInfo->firstAttributeInformation) == 0 && SlottedPage::getSlotID(relationInfo->firstAttributeInformation) == 0){
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
	TID attrTID = this->getAttribute(relationName, attributeName);
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	return attr->type;
}

unsigned int SchemaSegment::getAttributeOffset(string relationName, string attributeName){
	TID attrTID = this->getAttribute(relationName, attributeName);
	unique_ptr<AttributeInformation> attr = this->lookup<AttributeInformation>(attrTID);
	return attr->tupleOffset;
}


void SchemaSegment::readSchemaFromFile(string filename, SegmentManager* sm){
	// metadata/schema segment
	Parser p(filename);
	try {
		std::unique_ptr<Schema> schema = p.parse();
		//std::cout << schema->toString() << std::endl;

		for(const Schema::Relation& rel : schema->relations){
			// create segment and save relation information in schema segment
			SegmentID sid = sm->createSegment<SPSegment>(20);
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

			// TODO: index
		}
	} catch (ParserError& e) {
		std::cerr << e.what() << std::endl;
	}
}


TID SchemaSegment::schema(){
	return 0;
}

TID SchemaSegment::getRelation(string name){
	assert(name.size() <= LENGTH);
	TID relationTID = this->firstRelation();
	while( ! (SlottedPage::getPageID(relationTID) == 0 && SlottedPage::getSlotID(relationTID) == 0) ){
		unique_ptr<RelationInformation> info = this->lookup<RelationInformation>(relationTID);
		if(strcmp(info->name, name.c_str()) == 0){
			return relationTID;
		}
		relationTID = info->nextRelationInformation;
	}
	return relationTID;
}

TID SchemaSegment::firstRelation(){
	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schema());
	TID result = info->firstRelationInformation;
	return result;
}

TID SchemaSegment::lastRelation(){
	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schema());
	TID result = info->lastRelationInformation;
	return result;
}

bool SchemaSegment::isEmpty(){
	unique_ptr<SchemaInformation> info = this->lookup<SchemaInformation>(schema());
	TID last = info->lastRelationInformation;
	if(SlottedPage::getPageID(last) == 0 && SlottedPage::getSlotID(last) == 0){
		return true;
	} else {
		return false;
	}
}

TID SchemaSegment::getAttribute(string relationName, string attributeName){
	assert(attributeName.size() <= LENGTH);

	TID relationTID = this->getRelation(relationName);
	unique_ptr<RelationInformation> relation = this->lookup<RelationInformation>(relationTID);

	TID attributeTID = relation->firstAttributeInformation;
	while( ! (SlottedPage::getPageID(attributeTID) == 0 && SlottedPage::getSlotID(attributeTID) == 0) ){
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
