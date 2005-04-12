// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MUTEX_H
#define __MUTEX_H

namespace dingus {

// --------------------------------------------------------------------------

class CMutex : public boost::noncopyable {
public:
	CMutex();
	~CMutex();

	void	lock();
	void	unlock();

	static long		incrementRC( long* value );
	static long		decrementRC( long* value );

private:
	CRITICAL_SECTION	mCritSec;
};


// --------------------------------------------------------------------------

class CScopedLock : public boost::noncopyable {
public:
	CScopedLock( CMutex& mutex ) : mMutex(&mutex) { mMutex->lock(); }
	~CScopedLock() { mMutex->unlock(); }
private:
	CMutex*	mMutex;
};


} // lang


#endif // _LANG_MUTEX_H
