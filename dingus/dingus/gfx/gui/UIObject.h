// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_OBJECT_H
#define __UI_OBJECT_H


namespace dingus {


class IGUIObjectRenderer;

// --------------------------------------------------------------------------

/**
 *  Base UI widget.
 */
class CGUIObject : public CRefCounted {
public:
	typedef DingusSmartPtr<CGUIObject>	TSharedPtr;
public:
	CGUIObject( float x1, float y1, float x2, float y2 )
		: mX1(x1), mY1(y1), mX2(x2), mY2(y2), mVisible(true) { }
	virtual ~CGUIObject() { };

	virtual void render( IGUIObjectRenderer& renderer ) = 0;
	virtual void setPosition( float x1, float y1, float x2, float y2 ) {
		mX1 = x1; mY1 = y1; mX2 = x2; mY2 = y2;
	}

	bool	contains( float x, float y ) const {
		return mX1 <= x && mY1 <= y && mX2 >= x && mY2 >= y;
	}

	float	getX1() const { return mX1; }
	float	getY1() const { return mY1; }
	float	getX2() const { return mX2; }
	float	getY2() const { return mY2; }

	void setVisible( bool v ) { mVisible = v; }
	bool isVisible() const { return mVisible; }

private:
	/// Global coordinates
	float	mX1, mY1, mX2, mY2;

	/// Are we visible?
	bool		mVisible;
};


}; // namespace

#endif
