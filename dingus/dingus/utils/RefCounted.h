// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __CREFCOUNTED_H
#define __CREFCOUNTED_H


#define DingusSmartPtr boost::intrusive_ptr


namespace dingus {

/**
 *  Reference counted non-copyable object.
 *
 *  Has zero ref count when constructed.
 *  
 *  Intended for use with boost::intrusive_ptr.
 */
class CRefCounted : public boost::noncopyable {
public:
	CRefCounted() : mRefCount(0) { }
	virtual ~CRefCounted() { }

	/** Recrements refcount. Deletes self if refcount reaches zero. */
	int decRef();
	/** Increments refcount. */
	int incRef();
	/** Gets reference count. */
	int getRefCount() const { return mRefCount; }

private:
	int	mRefCount;
};


inline int CRefCounted::incRef() {
	return ++mRefCount;
}

inline int CRefCounted::decRef() {
	assert(mRefCount>0);
	int refs = --mRefCount;
	if(!mRefCount)
		delete this;
	return refs;
}

}; // namespace


//
// for boost::intrusive_ptr<clazz>

#define IMPLEMENT_INTRUSIVE_PTR(clazz) \
namespace boost { \
inline void intrusive_ptr_add_ref( clazz* p ) { p->incRef(); } \
inline void intrusive_ptr_release( clazz* p ) { p->decRef(); } \
}

IMPLEMENT_INTRUSIVE_PTR(dingus::CRefCounted);


#endif
