#include "SlottedPage.hpp"

SlottedPage::SlottedPage(BufferFrame& frame): _frame(frame) {
	_slotOccupation = reinterpret_cast<SLOTMASK*>(frame.getData());
}

Record* SlottedPage::getRecord(TID tid) {
	unsigned slotID = getSlotID(tid);
	if (!slotIsOccupied(slotID)) {
		return NULL;
	}

	char* slot = _slotPtr(slotID);
	size_t len = *reinterpret_cast<unsigned*>(slot);
	return new Record(len, slot+sizeof(unsigned));
}

TID* SlottedPage::getFreeSlot() {
	for (unsigned i=0; i<slotCount(); i++) {
		if (!slotIsOccupied(i)) {
			TID* tid = new TID;
			*tid = getTID(_frame.getPageId(), i);
			return tid;
		}
	}

	return NULL;
}

bool SlottedPage::slotIsOccupied(TID tid) {
	return (*_slotOccupation & (1<<getSlotID(tid)))!=0;
}

bool SlottedPage::occupySlot(TID slot, const Record& record) {
	unsigned slotID = getSlotID(slot);

	size_t size = sizeof(record.getLen())+record.getLen();
	if (size>slotSize(_frame)) {
		cerr<<size<<" > "<<slotSize(_frame)<<endl;
		throw RecordTooBigException;
	}

	*_slotOccupation |= 1<<slotID;
	*reinterpret_cast<unsigned*>(_slotPtr(slotID)) = record.getLen();
	memcpy(_slotPtr(slotID)+sizeof(unsigned), record.getData(), record.getLen());

	return true;
}

bool SlottedPage::freeSlot(TID slot)  {
	unsigned slotID = getSlotID(slot);

	*_slotOccupation &= ~(1<<slotID);
	return true;
}

unsigned SlottedPage::slotCount() {
	return sizeof(SLOTMASK)*8;
}

size_t SlottedPage::slotSize(BufferFrame& frame) {
	return (frame.getSize()-sizeof(SLOTMASK))/(sizeof(SLOTMASK)*8);
}
unsigned SlottedPage::getPageID(TID tid) {
	return tid >> (sizeof(SLOTMASK)*8);
}
unsigned SlottedPage::getSlotID(TID tid) {
	return tid & static_cast<unsigned>(pow(2,sizeof(SLOTMASK)*8)-1);
}
TID SlottedPage::getTID(unsigned pageID, unsigned slotID) {
	return (pageID<<((sizeof(SLOTMASK)*8))
						| (slotID & ~(1<<sizeof(SLOTMASK)*8)));
}


char* SlottedPage::_slotPtr(unsigned slotID) {
	return reinterpret_cast<char*>(_frame.getData()) + sizeof(SLOTMASK) + slotSize(_frame)*slotID;
}
