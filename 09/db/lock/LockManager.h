#ifndef LOCKMANAGER_H_
#define LOCKMANAGER_H_

#include "../segments/datastructures.h"
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <list>
#include <sstream>

static const int DEBUG = 1;

using namespace std;

template <class T>
inline string to_string(const T& t){
	stringstream ss;
	ss << t;
	return ss.str();
}

typedef unsigned int TxID;
//struct Lock {int key; int owner; int status;};
//struct Tx {TxID id; Tx* nextTuple; Tx* nextTx;};

class LockManager {
private:
	map<TID, list<TxID>, TID> tuples; 	// chain of transactions that wait for each tuple
	map<TxID, list<TID>> txs;			// tuples for each transaction
	map<TID, pthread_rwlock_t, TID> locks; // lock for each tuple -> TODO: maybe save locks in Tx, but then one would have to go through all transactions
	pthread_mutex_t printLock;
	pthread_mutex_t mapLock;			// really necessary?

public:
	LockManager();
	virtual ~LockManager();

	string toStr(TID tid, TxID tx){
		stringstream ss;
		ss << "Tx: " << tx << ", TID: " << tid.slotID << tid.pageID << " > ";
		return ss.str();
	}

	string strWaiting(TID tid){
		stringstream ss;
		ss << "[";
		bool first = true;
		for(TxID tx : tuples[tid]){
			if(!first) ss << ", ";
			first = false;
			ss << tx;
		}
		ss << "]";
		return ss.str();
	}

	void debug(string s){
		if(!DEBUG) return;
		pthread_mutex_lock(&printLock);
		cout << s << endl;
		pthread_mutex_unlock(&printLock);
	}

	// strict 2PL: if locked -> wait for transaction
	int lock(TID tid, TxID tx){
		string s = toStr(tid, tx);
		if(!locks.count(tid)){ // init lock
//			cout << s << "no lock yet -> init" << endl;
			debug(s + "no lock yet -> init");
			pthread_mutex_lock(&mapLock);
			pthread_rwlock_init(&locks[tid], NULL);
			pthread_mutex_unlock(&mapLock);
		}
		if(tuples.count(tid) && !tuples[tid].empty()){ // already some transaction(s) waiting for tid
			debug(s + "there are already " + to_string(tuples[tid].size()) + " transactions waiting: " + strWaiting(tid));
		}else if(locks.count(tid)){ // lock initialized but tid not locked yet
			debug(s + "not locked yet");
		}
		// acquire lock
//		pthread_rwlock_wrlock(&locks[tid]); // lock tid -> will wait if already locked
		int status = pthread_rwlock_trywrlock(&locks[tid]); // try to lock tid -> will abort if already locked
		if(status != 0){
			debug(s + "couldn't acquire lock -> abort");
			return 0;
		}
		debug(s + "successfully acquired lock");
		pthread_mutex_lock(&mapLock);
		tuples[tid].push_back(tx); // add tx to queue for tid
		txs[tx].push_back(tid); // add tid to queue for tx
		pthread_mutex_unlock(&mapLock);
//		return tuples[tid].size();
		return 1;
	}

	void unlockAll(TxID tx){
		// go through all tids that tx locks
		pthread_mutex_lock(&mapLock);
		for(TID tid : txs[tx]){
			tuples[tid].remove(tx); // and remove tx
			pthread_rwlock_unlock(&locks[tid]); // unlock tid
			pthread_rwlock_destroy(&locks[tid]); // free it?
			debug(toStr(tid, tx) + "successfully released lock");
		}
		txs.erase(tx); // tx is done
		pthread_mutex_unlock(&mapLock);
//		cout << "unlocked all tids for Tx " << tx << endl;
	}

};

#endif /* LOCKMANAGER_H_ */
