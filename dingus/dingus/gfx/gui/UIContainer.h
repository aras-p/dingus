// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_CONTAINER_H
#define __UI_CONTAINER_H

#include "UIObject.h"

namespace dingus {

typedef fastvector<CGUIObject::TSharedPtr>	TGUIObjectVector;
typedef TGUIObjectVector::const_iterator	TGUIObjectCIterator;


// --------------------------------------------------------------------------

// Base container - can't use directly.
// Just provides non-templated interface to policies.
class CGUIBaseContainer : public CGUIObject {
public:
	void addObject( CGUIObject& o ) { mObjects.push_back( &o ); }
	const TGUIObjectVector& getObjects() const { return mObjects; }

	// CGUIObject
	virtual void render( IGUIObjectRenderer& renderer ) {
		TGUIObjectVector::iterator it, itEnd = mObjects.end();
		for( it = mObjects.begin(); it != itEnd; ++it ) {
			CGUIObject* o = it->get();
			assert( o );
			o->render( renderer );
		}
	}
protected:
	CGUIBaseContainer( float x1, float y1, float x2, float y2 ) : CGUIObject(x1,y1,x2,y2) { }
private:
	TGUIObjectVector	mObjects;
};


// --------------------------------------------------------------------------

/**
 *  An add-only UI objects container.
 *
 *  You can add UI objects to it, and it renders them in the order of
 *  addition. It also routes setPosition() calls through resizing policy.
 *
 *  Policy must be:
 *	-	Default constructable
 *	-	void resize( float x1, float y1, float x2, float y2, const CUIBaseContainer& me );
 *		This is called on setPosition() call, passing new coordinates and
 *		*this object. Note that the new coordinates are not yet set (the
 *		container still has old coordinates).
 */
template< typename RESIZE_POLICY >
class CGUIContainer : public CGUIBaseContainer, public RESIZE_POLICY {
public:
	CGUIContainer( float x1, float y1, float x2, float y2 ) : CUIBaseContainer(x1,y1,x2,y2) { }

	// CGUIObject
	virtual void setPosition( float x1, float y1, float x2, float y2 ) {
		// do resizing in policy...
		// don't change our size yet - give policy new size and let it get
		// old size if it needs
		resize( x1, y1, x2, y2, *this );

		// set our size finally
		CGUIObject::setPosition( x1, y2, x2, y2 );
	}
};

}; // namespace

#endif
