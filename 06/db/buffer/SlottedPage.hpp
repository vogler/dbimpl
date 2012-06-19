#ifndef SLOTTEDPAGE_HPP_
#define SLOTTEDPAGE_HPP_

#include "BufferFrame.hpp"
#include "Record.hpp"
#include <exception>
#include <math.h>
using namespace std;


typedef uint64_t TID;
typedef uint8_t SLOTMASK;


/**
 * Slotted Page
 */
class SlottedPage
{
	static class RecordTooBigException: public exception
	{
	} RecordTooBigException;


public:
	SlottedPage(BufferFrame& frame);


	Record* getRecord(TID tid);
	TID* getFreeSlot();
	bool slotIsOccupied(TID tid);

	bool occupySlot(TID slot, const Record& record);
	bool freeSlot(TID slot);

	unsigned slotCount();

	static size_t slotSize(BufferFrame& frame);
	static unsigned getPageID(TID tid);
	static unsigned getSlotID(TID tid);
	static TID getTID(unsigned pageID, unsigned slotID);


protected:
	BufferFrame& _frame;
	SLOTMASK* _slotOccupation;

	char* _slotPtr(unsigned slotID);
};


#endif /* SLOTTEDPAGE_HPP_ */
