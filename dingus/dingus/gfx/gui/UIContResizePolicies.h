// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_CONTAINER_RESIZE_POLICIES_H
#define __UI_CONTAINER_RESIZE_POLICIES_H

#include "UIContainer.h"

namespace dingus {
namespace ui {

// Some common policies for UI container resizing.
// See comments at CUIContainer for policy requirements and semantics.

struct cont_resize_policy_empty {
protected:
	static void resize( float x1, float y1, float x2, float y2, const CUIBaseContainer& me ) {
	}
};

struct cont_resize_policy_expand {
protected:
	static void resize( float x1, float y1, float x2, float y2, const CUIBaseContainer& me ) {
		const TUIObjectVector& objs = me.getObjects();
		TUIObjectCIterator it, itEnd = objs.end();
		for( it = objs.begin(); it != itEnd; ++it ) {
			CGUIObject* o = it->get();
			assert( o );
			o->setPosition( x1, y1, x2, y2 );
		}
	}
};

struct cont_resize_policy_translate {
protected:
	static void resize( float x1, float y1, float x2, float y2, const CUIBaseContainer& me ) {
		assert( x2-x1 == me.getX2()-me.getX1() );
		assert( y2-y1 == me.getY2()-me.getY1() );
		const float dx = x1 - me.getX1();
		const float dy = y1 - me.getY1();
		const TUIObjectVector& objs = me.getObjects();
		TUIObjectCIterator it, itEnd = objs.end();
		for( it = objs.begin(); it != itEnd; ++it ) {
			CGUIObject* o = it->get();
			assert( o );
			o->setPosition( o->getX1()+dx, o->getY1()+dy, o->getX2()+dx, o->getY2()+dy );
		}
	}
};

template< float spaceX1, float spaceX2, float spaceY = 0.0f >
struct cont_resize_policy_column {
protected:
	static void resize( float x1, float y1, float x2, float y2, const CUIBaseContainer& me ) {
		float y = y1;
		const TUIObjectVector& objs = me.getObjects();
		TUIObjectCIterator it, itEnd = objs.end();
		for( it = objs.begin(); it != itEnd; ++it ) {
			CGUIObject* o = it->get();
			assert( o );
			const float ody = o->getY2() - o->getY1();
			o->setPosition( x1+spaceX1, y, x2-spaceX2, y+ody );
			y += ody + spaceY;
		}
	}
};

}; // namespace ui
}; // namespace dingus

#endif
