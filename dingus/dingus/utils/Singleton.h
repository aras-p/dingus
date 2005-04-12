// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SINGLETON_H
#define __SINGLETON_H

namespace dingus {

template<typename T>
class CSingleton {
public:
	static T& getInstance() {
		if( !isCreated() )
			mSingleInstance = T::createInstance();
		return *mSingleInstance;
	}

	static void finalize() {
		if( !isCreated() )
			return;
		T::deleteInstance( *mSingleInstance );
		mSingleInstance = 0;
	}

protected:
	static void assignInstance( T& instance ) { mSingleInstance = &instance; }
	static bool isCreated() { return ( mSingleInstance != NULL ); }
	CSingleton() { }

private:
	static T*	mSingleInstance;
};


template< typename T >
T* CSingleton<T>::mSingleInstance = 0;


#define IMPLEMENT_SIMPLE_SINGLETON(clazz) \
	static clazz* createInstance() { return new clazz(); } \
	static void deleteInstance( clazz& o ) { delete &o; } \
	friend class dingus::CSingleton<clazz>


}; // namespace

#endif
