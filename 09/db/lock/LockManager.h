#ifndef LOCKMANAGER_H_
#define LOCKMANAGER_H_

#include "../segments/datastructures.h"
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <list>
#include <sstream>

using namespace std;

typedef unsigned int TxID;
//struct Lock {int key; int owner; int status;};
//struct Tx {TxID id; Tx* nextTuple; Tx* nextTx;};

class LockManager {
private:
	map<TID, list<TxID>, TID> tuples; 	// lock chain of transactions that wait for each tuple
	map<TxID, list<TID>> txs;		// tuples for each transaction
	map<TID, pthread_rwlock_t, TID> locks; // lock for each tuple

public:
	LockManager();
	virtual ~LockManager();

	string toS(TID tid, TxID tx){
		stringstream ss;
		ss << "TID: " << tid.slotID << tid.pageID << ", Tx: " << tx << " > ";
		return ss.str();
	}

	// strict 2PL: if locked -> wait for transaction
	int lock(TID tid, TxID tx){
		string s = toS(tid, tx);
		if(!locks.count(tid)){ // init lock
			cout << s << "no lock yet -> init" << endl;
			pthread_rwlock_init(&locks[tid], NULL);
		}
		if(tuples.count(tid) && !tuples[tid].empty()){ // already some transaction(s) waiting for tid
			cout << s << "there are already " << tuples[tid].size() << " transactions waiting" << endl;
		}else{ // tid not locked yet
			cout << s << "not locked yet" << endl;
		}
		// acquire lock
		pthread_rwlock_wrlock(&locks[tid]); // lock tid -> will wait if already locked
		cout << s << "successfully acquired lock" << endl;
		tuples[tid].push_back(tx); // add tx to queue for tid
		txs[tx].push_back(tid); // add tid to queue for tx
		return tuples[tid].size();
	}

	void unlockAll(TxID tx){
		// go through all tids that tx locks
		for(TID tid : txs[tx]){
			tuples[tid].remove(tx); // and remove tx
			pthread_rwlock_unlock(&locks[tid]); // unlock tid
			cout << toS(tid, tx) << "successfully released lock" << endl;
		}
		txs.erase(tx); // tx is done
//		cout << "unlocked all tids for Tx " << tx << endl;
	}

};

#endif /* LOCKMANAGER_H_ */
