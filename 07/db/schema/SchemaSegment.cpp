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

	SRelation rel = { "", segment, 0, 0, 0 };
	name.copy(rel.name, name.size(), 0);
	TID newRel = this->insert<SRelation>(rel);

	unique_ptr<SSchema> info = this->lookup<SSchema>(0);
	if(this->isEmpty()){ // first
		info->firstRelation = newRel;
	} else { // next & last
		unique_ptr<SRelation> currentLast = this->lookup<SRelation>(info->lastRelation);
		currentLast->nextRelation = newRel;
		this->update<SRelation>(info->lastRelation, move(currentLast));
	}
	info->lastRelation = newRel;
	this->update<SSchema>(0, move(info));
	return newRel;
}

SegmentID SchemaSegment::getRelationSegmentID(string relationName){
	TID relationInfoTID = this->getRelation(relationName);
	unique_ptr<SRelation> relationInfo = this->lookup<SRelation>(relationInfoTID);
	return relationInfo->segment;
}

TID SchemaSegment::addAttribute(string relationName, string attributeName,
		unsigned int tupleOffset, Types::Tag type, unsigned int length, bool notNull){
	assert(relationName.size() <= LENGTH && attributeName.size() <= LENGTH);

	TID relationInfoTID = this->getRelation(relationName);
	unique_ptr<SRelation> relationInfo = this->lookup<SRelation>(relationInfoTID);

	SAttribute newAttrRec = { "", tupleOffset, type, length, notNull, 0, false, 0 };
	attributeName.copy(newAttrRec.name, attributeName.size(), 0);
	TID newAttr = this->insert<SAttribute>( newAttrRec );

	if(SlottedPage::getPageID(relationInfo->firstAttribute) == 0 && SlottedPage::getSlotID(relationInfo->firstAttribute) == 0){
		relationInfo->firstAttribute = newAttr;
	} else {
		unique_ptr<SAttribute> currentLast = this->lookup<SAttribute>(relationInfo->lastAttribute);
		currentLast->nextAttribute = newAttr;
		this->update<SAttribute>(relationInfo->lastAttribute, move(currentLast));
	}
	relationInfo->lastAttribute = newAttr;
	this->update<SRelation>(relationInfoTID, move(relationInfo));
	return newAttr;
}

Types::Tag SchemaSegment::getAttributeType(string relationName, string attributeName){
	TID attrTID = this->getAttribute(relationName, attributeName);
	unique_ptr<SAttribute> attr = this->lookup<SAttribute>(attrTID);
	return attr->type;
}

unsigned int SchemaSegment::getAttributeOffset(string relationName, string attributeName){
	TID attrTID = this->getAttribute(relationName, attributeName);
	unique_ptr<SAttribute> attr = this->lookup<SAttribute>(attrTID);
	return attr->offset;
}

void SchemaSegment::readSchemaFromFile(string filename, SegmentManager* sm){
	Parser p(filename);
	try {
		std::unique_ptr<Schema> schema = p.parse();

		for(const Schema::Relation& rel : schema->relations){
			SegmentID sid = sm->createSegment<SPSegment>(20);
			this->addRelation(rel.name, sid);

			unsigned int tupleOffset = 0;
			for (const auto& attr : rel.attributes) {
				this->addAttribute(rel.name, attr.name, tupleOffset, attr.type, attr.len, attr.notNull);
				if(attr.len != (unsigned int)-1)
					tupleOffset += attr.len;
				else
					tupleOffset += 4;
			}
		}
	} catch (ParserError& e) {
		std::cerr << e.what() << std::endl;
	}
}

TID SchemaSegment::getRelation(string name){
	assert(name.size() <= LENGTH);
	TID relationTID = this->firstRelation();
	while( ! (SlottedPage::getPageID(relationTID) == 0 && SlottedPage::getSlotID(relationTID) == 0) ){
		unique_ptr<SRelation> info = this->lookup<SRelation>(relationTID);
		if(strcmp(info->name, name.c_str()) == 0){
			return relationTID;
		}
		relationTID = info->nextRelation;
	}
	return relationTID;
}

TID SchemaSegment::firstRelation(){
	unique_ptr<SSchema> info = this->lookup<SSchema>(0);
	TID result = info->firstRelation;
	return result;
}

TID SchemaSegment::lastRelation(){
	unique_ptr<SSchema> info = this->lookup<SSchema>(0);
	TID result = info->lastRelation;
	return result;
}

bool SchemaSegment::isEmpty(){
	unique_ptr<SSchema> info = this->lookup<SSchema>(0);
	TID last = info->lastRelation;
	if(SlottedPage::getPageID(last) == 0 && SlottedPage::getSlotID(last) == 0){
		return true;
	} else {
		return false;
	}
}

TID SchemaSegment::getAttribute(string relationName, string attributeName){
	assert(attributeName.size() <= LENGTH);

	TID relationTID = this->getRelation(relationName);
	unique_ptr<SRelation> relation = this->lookup<SRelation>(relationTID);

	TID attributeTID = relation->firstAttribute;
	while( ! (SlottedPage::getPageID(attributeTID) == 0 && SlottedPage::getSlotID(attributeTID) == 0) ){
		unique_ptr<SAttribute> info = this->lookup<SAttribute>(attributeTID);
		if(strcmp(info->name, attributeName.c_str()) == 0){
			return attributeTID;
		}
		attributeTID = info->nextAttribute;
	}
	return attributeTID;
}

} /* namespace std */
