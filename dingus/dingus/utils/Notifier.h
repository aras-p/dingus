// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __NOTIFIER_H
#define __NOTIFIER_H

namespace dingus {

/**
 *  Notifier class. Provides listener registry, listener notification
 *  is left to subclasses.
 *
 *  Note that it doesn't have virtual functionality at all
 *  (eg. no virtual destructor).
 */
template<typename T>
class CNotifier {
public:
	typedef fastvector<T*>	TListenerVector;
public:
	void addListener( T& listener ) { mListeners.push_back( &listener ); }
	void removeListener( T& listener ) { mListeners.remove( &listener ); }
	void clearListeners() { mListeners.clear(); }

	const TListenerVector& getListeners() const { return mListeners; }
	TListenerVector& getListeners() { return mListeners; }

private:
	TListenerVector mListeners;
};


}; // namespace

#endif
