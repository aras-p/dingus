// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __INPUT_EVENT_H
#define __INPUT_EVENT_H

#define DIRECTINPUT_VERSION         0x0800
#include <dinput.h>


namespace dingus {

// --------------------------------------------------------------------------

class CInputEvent : public CRefCounted {
public:
	enum { EVENT_TYPE = 0 };
	typedef DingusSmartPtr<CInputEvent>	TSharedPtr;
	
public:
	virtual ~CInputEvent() = 0 { }

	virtual bool operator< ( CInputEvent const& rhs ) const;	
	
	int getType() const { return mType; };

protected:
	CInputEvent( int type ) : mType(type) { }

	static int unique( const CInputEvent& event );
	
private:
	int		mType;
};



// --------------------------------------------------------------------------

class CKeyEvent : public CInputEvent {
public:
	enum { EVENT_TYPE = 1 };
	enum eMode { KEY_PRESSED, KEY_DOWN, KEY_RELEASED };
	
public:
	CKeyEvent( eMode mode, int ascii, int keyCode = 0 )
		: CInputEvent(EVENT_TYPE), mMode(mode), mAscii(ascii), mKeyCode(keyCode) { }

	virtual bool operator< ( CInputEvent const& rhs ) const;	
	
	/// Event mode (pressed, down, released).
	eMode	getMode() const { return mMode; };
	/// Ascii code (if a key has it).
	int		getAscii() const { return mAscii; };
	/// DInput key codes (DIK_*).
	int		getKeyCode() const { return mKeyCode; };

protected:
	static int unique( CKeyEvent const& event, bool withSpecialKey );

private:
	eMode	mMode;
	int		mAscii;
	int		mKeyCode;
};



// --------------------------------------------------------------------------

class CMouseEvent : public CInputEvent {
public:
	enum { EVENT_TYPE = 2 };
	enum eMode { MOUSE_PRESSED, MOUSE_RELEASED, MOUSE_DRAGGED, MOUSE_DOWN, MOUSE_MOVE, MOUSE_NO_ACTION };
	enum eMouseButton { NONE = 0, LEFT_BTN = 1, RIGHT_BTN = 2, MIDDLE_BTN = 4 };
public:
	CMouseEvent( eMode mode, eMouseButton button )
		: CInputEvent(EVENT_TYPE), mMode(mode), mButton(button), mX(0), mY(0), mClickCount(1) { }
	CMouseEvent( eMode mode, eMouseButton button, int x, int y, int clickCount = 1 )
		: CInputEvent(EVENT_TYPE), mMode(mode), mButton(button), mX(x), mY(y), mClickCount(clickCount) { }


	virtual bool operator< ( CInputEvent const& rhs ) const;	
	
	eMode			getMode() const { return mMode; };
	eMouseButton	getButton() const { return mButton; };
	int				getX() const { return mX; };
	int				getY() const { return mY; };
	int				getClickCount() const { return mClickCount; };

protected:
	static int unique( CMouseEvent const& event );
	
private:
	eMode			mMode;
	eMouseButton	mButton;
	int				mX;
	int				mY;
	int				mClickCount;
};



// --------------------------------------------------------------------------

class CInputEvents {
public:
	typedef std::vector<CInputEvent::TSharedPtr>	TEventsVector;

public:
	CInputEvents() { };
	CInputEvents( CInputEvents const& events ) : mEvents(events.mEvents) { };

	void addEvent( CInputEvent* event ) { mEvents.push_back(event); }
	TEventsVector const& getEvents() const { return mEvents; }
	void clear() { mEvents.clear(); }

private:
	TEventsVector	mEvents;
};


}; // namespace


#endif