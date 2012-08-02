#include "LockManager.h"

LockManager::LockManager() {
	// TODO Auto-generated constructor stub
	printLock = PTHREAD_MUTEX_INITIALIZER;
	mapLock = PTHREAD_MUTEX_INITIALIZER;
}

LockManager::~LockManager() {
	// TODO Auto-generated destructor stub
}

