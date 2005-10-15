// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DINGUS_GUI_H
#define __DINGUS_GUI_H

#include "../../kernel/D3DDevice.h"
#include "../../kernel/Proxies.h"
#include "../../resource/DeviceResource.h"
#include "../../math/FPU.h"

namespace dingus {


// --------------------------------------------------------------------------
//  Constants

const int UI_MAX_CTRL_STATES = 16;


#define UIEVENT_BUTTON_CLICKED				0x0101
#define UIEVENT_COMBOBOX_SELECTION_CHANGED	0x0201
#define UIEVENT_RADIOBUTTON_CHANGED			0x0301
#define UIEVENT_CHECKBOX_CHANGED				0x0401
#define UIEVENT_SLIDER_VALUE_CHANGED			0x0501
// UIEVENT_EDITBOX_CHANGE is sent when the listbox content changes
// due to user input.
#define UIEVENT_EDITBOX_CHANGE				0x0602
#define UIEVENT_LISTBOX_ITEM_DBLCLK			0x0701
// UIEVENT_LISTBOX_SELECTION is fired off when the selection changes in
// a single selection list box.
#define UIEVENT_LISTBOX_SELECTION 			0x0702


// --------------------------------------------------------------------------
//  Forward declarations

class CUIDialog;
class CUIControl;
class CUIButton;
class CUIStatic;
class CUIImage;
class CUICheckBox;
class CUIRadioButton;
class CUIComboBox;
class CUISlider;
class CUIListBox;
class CUIScrollBar;
class CUIRollout;
class SUIElement;
struct SUIFontNode;

typedef void (CALLBACK *TUICallbackEvent)( UINT evt, int ctrlID, CUIControl* ctrl );
typedef void (CALLBACK *TUICallbackRender)( CUIDialog& dlg );


// --------------------------------------------------------------------------
//  Enums for pre-defined control types

enum eUIControlType { 
	UICTRL_BUTTON, 
	UICTRL_STATIC, 
	UICTRL_CHECKBOX,
	UICTRL_RADIOBUTTON,
	UICTRL_COMBOBOX,
	UICTRL_SLIDER,
	UICTRL_LISTBOX,
	UICTRL_SCROLLBAR,
	UICTRL_ROLLOUT,
	UICTRL_IMAGE,
};

enum eUIControlState {
	UISTATE_NORMAL,
	UISTATE_DISABLED,
	UISTATE_HIDDEN,
	UISTATE_FOCUS,
	UISTATE_MOUSEOVER,
	UISTATE_PRESSED,
};

// --------------------------------------------------------------------------

static inline RECT makeRECT( int l, int t, int r, int b )
{
	RECT rc = { l, t, r, b };
	return rc;
}

struct SFPoint {
	float	x;
	float	y;
};

struct SFRect {
public:
	SFRect() { } // empty constructor
	SFRect( float l, float t, float r, float b )
		: left(l), top(t), right(r), bottom(b) { }
	
	float	left;
    float	top;
    float	right;
    float	bottom;
public:
	void	set( float l, float t, float r, float b ) {
		left = l; top = t; right = r; bottom = b;
	}
	bool	containsPoint( const POINT& pt ) const {
		return pt.x >= left && pt.y >= top && pt.x < right && pt.y < bottom;
	}
	void	offset( float dx, float dy ) {
		left += dx;		top += dy;
		right += dx;	bottom += dy;
	}
	void	inflate( float dx, float dy ) {
		left -= dx;		top -= dy;
		right += dx;	bottom += dy;
		if( left > right )
			left = right = (left+right)*0.5f;
		if( top > bottom )
			top = bottom = (top+bottom)*0.5f;
	}
	void	toRect( RECT& rc ) const {
		rc.left		= dingus::round( left );
		rc.top		= dingus::round( top );
		rc.right	= dingus::round( right );
		rc.bottom	= dingus::round( bottom );
	}
	float	getWidth() const { return right - left; }
	float	getHeight() const { return bottom - top; }
};

// --------------------------------------------------------------------------

struct SBlendColor {
	void init( bool font, bool darkFont = false );
	void blend( UINT state, float dt, float rate = 0.7f );
	
	D3DCOLOR  colors[ UI_MAX_CTRL_STATES ]; // Modulate colors for all possible control states
	D3DXCOLOR current;
};


// --------------------------------------------------------------------------

/// Contains all the display tweakables for a sub-control
class SUIElement {
public:
	void setTexture( CD3DTexture* tex, const RECT* texRect );
	void setFont( UINT font, bool dark, DWORD textFmt = DT_CENTER | DT_VCENTER );
	
	void refresh();
	
	SBlendColor colorTexture;
	SBlendColor colorFont;

	RECT	textureRect; 	// Bounding rect of this element on the composite texture

	CD3DTexture* texture;	// Texture for this element 
	UINT	fontIdx; 		// Index of the font for this element
	DWORD	textFormat; 	// The format argument to drawText 
	bool	darkFont;		// Is the text dark (no need for the shadow)?
};


// --------------------------------------------------------------------------

///  All controls must be assigned to a dialog, which handles input and rendering for the controls.
class CUIDialog : public boost::noncopyable {
public:
	CUIDialog();
	~CUIDialog();

	// Windows message handler
	bool msgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	// Control creation
	HRESULT addStatic( int cid, const char* text, int x, int y, int width, int height, bool isDefault=false, CUIStatic** created=NULL );
	HRESULT addButton( int cid, const char* text, int x, int y, int width, int height, UINT hotkey=0, bool isDefault=false, CUIButton** created=NULL );
	HRESULT addCheckBox( int cid, const char* text, int x, int y, int width, int height, bool chk=false, UINT hotkey=0, bool isDefault=false, CUICheckBox** created=NULL );
	HRESULT addRadioButton( int cid, UINT group, const char* text, int x, int y, int width, int height, bool chk=false, UINT hotkey=0, bool isDefault=false, CUIRadioButton** created=NULL );
	HRESULT addComboBox( int cid, int x, int y, int width, int height, UINT nHotKey=0, bool isDefault=false, CUIComboBox** created=NULL );
	HRESULT addSlider( int cid, int x, int y, int width, int height, int min=0, int max=100, int value=50, bool isDefault=false, CUISlider** created=NULL );
	HRESULT addListBox( int cid, int x, int y, int width, int height, DWORD style=0, CUIListBox** created=NULL );
	HRESULT addRollout( int cid, const char* text, int x, int y, int width, int height, int rollHeight, bool expanded=false, UINT hotkey=0, bool isDefault=false, CUIRollout** created=NULL );
	HRESULT addImage( int cid, int x, int y, int width, int height, CD3DTexture& tex, int u0, int v0, int u1, int v1, CUIImage** created=NULL );
	HRESULT addControl( CUIControl* ctrl );
	HRESULT initControl( CUIControl* ctrl );

	// Control retrieval
	CUIStatic*		getStatic( int cid ) { return (CUIStatic*) getControl( cid, UICTRL_STATIC ); }
	CUIButton*		getButton( int cid ) { return (CUIButton*) getControl( cid, UICTRL_BUTTON ); }
	CUICheckBox*	getCheckBox( int cid ) { return (CUICheckBox*) getControl( cid, UICTRL_CHECKBOX ); }
	CUIRadioButton*	getRadioButton( int cid ) { return (CUIRadioButton*) getControl( cid, UICTRL_RADIOBUTTON ); }
	CUIComboBox*	getComboBox( int cid ) { return (CUIComboBox*) getControl( cid, UICTRL_COMBOBOX ); }
	CUISlider*		getSlider( int cid ) { return (CUISlider*) getControl( cid, UICTRL_SLIDER ); }
	CUIListBox*		getListBox( int cid ) { return (CUIListBox*) getControl( cid, UICTRL_LISTBOX ); }
	CUIRollout*		getRollout( int cid ) { return (CUIRollout*) getControl( cid, UICTRL_ROLLOUT ); }
	CUIImage*		getImage( int cid ) { return (CUIImage*) getControl( cid, UICTRL_IMAGE ); }

	CUIControl* getControl( int cid );
	CUIControl* getControl( int cid, eUIControlType ctrlType );
	CUIControl* getControlAtPoint( POINT pt );

	bool isControlEnabled( int cid );
	void setControlEnabled( int cid, bool enabled );

	void clearRadioButtonGroup( UINT nGroup );
	void clearComboBox( int cid );

	// Access the default display Elements used when adding new controls
	HRESULT 	setDefaultElement( eUIControlType ctrlType, UINT index, SUIElement* element );
	SUIElement* getDefaultElement( eUIControlType ctrlType, UINT index );

	// Methods called by controls
	void sendEvent( UINT evt, bool userTriggered, CUIControl* ctrl );
	void requestFocus( CUIControl* ctrl );

	// render helpers
	HRESULT drawRect( const SFRect* rect, D3DCOLOR color );
	//HRESULT drawPolyLine( const POINT* points, UINT pointCount, D3DCOLOR color );
	HRESULT drawSprite( SUIElement* element, const SFRect* dest );
	//HRESULT calcTextRect( const char* text, SUIElement* element, SFRect* dest, int count = -1 );
	HRESULT drawText( const char* text, SUIElement* element, const SFRect* dest, bool shadow = false, int count = -1 );
	HRESULT drawText( const wchar_t* text, SUIElement* element, const SFRect* dest, bool shadow = false, int count = -1 );

	// immediate mode renderers
	HRESULT imDrawSprite( const D3DXCOLOR& color, const RECT& texRect, CD3DTexture* tex, const SFRect& destScr );
	HRESULT imDrawText( const char* text, UINT fontIdx, DWORD format, const D3DXCOLOR& color, const SFRect& destScr, bool shadow = false, int count = -1 );
	HRESULT imDrawText( const wchar_t* text, UINT fontIdx, DWORD format, const D3DXCOLOR& color, const SFRect& destScr, bool shadow = false, int count = -1 );

	// Attributes
	bool isMinimized() const { return mMinimized; }
	void setMinimized( bool minimized ) { mMinimized = minimized; }
	void setBackgroundColors( D3DCOLOR colorAllCorners ) { setBackgroundColors( colorAllCorners, colorAllCorners, colorAllCorners, colorAllCorners ); }
	void setBackgroundColors( D3DCOLOR colorUL, D3DCOLOR colorUR, D3DCOLOR colorDL, D3DCOLOR colorDR );
	void enableCaption( bool benable ) { mHasCaption = benable; }
	float getCaptionHeight() const { return mCaptionHeight; }
	void setCaptionHeight( float height ) { mCaptionHeight = height; }
	void setCaptionText( const char *text ) { strncpy( mCaption, text, sizeof(mCaption)/sizeof(mCaption[0]) ); mCaption[sizeof(mCaption)/sizeof(mCaption[0])-1] = 0; }
	//void getLocation( POINT& pt ) const { pt.x = mX; pt.y = mY; }
	void setLocation( float x, float y ) { mX = x; mY = y; }
	void setSize( float width, float height ) { mWidth = width; mHeight = height;  }

	float getX() const { return mX; }
	float getY() const { return mY; }
	float getWidth() const { return mWidth; }
	float getHeight() const { return mHeight; }

	void setNextDialog( CUIDialog* nextDlg );

	static void setRefreshTime( float fTime ) { sTimeRefresh = fTime; }

	static CUIControl* getNextControl( CUIControl* ctrl );
	static CUIControl* getPrevControl( CUIControl* ctrl );

	void removeControl( int cid );
	void removeAllControls();

	// sets the callback used to notify the app of control events
	void setCallback( TUICallbackEvent pCallback ) { mCallbackEvent = pCallback; }
	void enableNonUserEvents( bool benable ) { mDoNonUserEvents = benable; }
	void enableKeyboardInput( bool benable ) { mDoKeyboardInput = benable; }
	void enableMouseInput( bool benable ) { mDoMouseInput = benable; }


	void	refresh();

	/**
	 *  Render whole dialog and controls. This calls renderBegin() and
	 *  renderEnd() internally; also calls render callback if any.
	 */
	void	onRender( float dt );

	/**
	 *  Manually prepare for dialog rendering. This sets all required
	 *  rendering state; after this call drawYYY and imDrawYYY methods can
	 *  be used. Use in cases where you want to render something UI-related,
	 *  but probably don't want to draw controls, etc.
	 */
	void	renderBegin( bool renderDlgBackground = false );

	/**
	 *	End manual dialog rendering.
	 */
	void	renderEnd();


	void	setRenderCallback( TUICallbackRender renderCb ) { mRenderCallback = renderCb; }

	
	// Shared resource access.
	void setFont( UINT index, const char* facename, LONG height, LONG weight );
	SUIFontNode* getFont( UINT index ) const;
	void	setDefaultTexture( CD3DTexture& texture ) { mDefaultTexture = &texture; }


	static void clearFocus();
	void focusDefaultControl();

	/// True if there is a control is in focus, belongs to this dialog, and is enabled.
	bool	hasFocusControl() const;

	bool mDoNonUserEvents;
	bool mDoKeyboardInput;
	bool mDoMouseInput;

private:
	// initialize default Elements
	void initDefaultElements();

	// Windows message handlers
	void onMouseMove( POINT pt );
	void onMouseUp( POINT pt );

	// Control events
	void onCycleFocus( bool forward );
	void onMouseEnter( CUIControl* ctrl );
	void onMouseLeave( CUIControl* ctrl );

private:
	struct SElementHolder {
		SUIElement		element;
		eUIControlType	ctrlType;
		int				index;
	};

private:
	static CUIControl* sCtrlFocus;		 // The control which has focus
	static CUIControl* sCtrlPressed; 	 // The control currently pressed

	char	mCaption[256];
	bool	mHasCaption;
	bool	mMinimized;

	float	mX;
	float	mY;
	float	mWidth;
	float	mHeight;
	float	mCaptionHeight;

	D3DCOLOR mColorUL;
	D3DCOLOR mColorUR;
	D3DCOLOR mColorDL;
	D3DCOLOR mColorDR;

	TUICallbackEvent mCallbackEvent;

	CD3DTexture*	mDefaultTexture;
	std::vector< int > mFonts; 	 // Index into mFontCache;

	std::vector< CUIControl* >	mControls;
	std::vector< SElementHolder* >	mDefaultElements;

	SUIElement mCaptionElement;	// element for the caption

	/**
	 *  Callback called inside rendering (before rendering controls), when
	 *  all dialog render states are applied. Here one can arbitrarily
	 *  call dialog's draw* functions (but otherwise don't mess with device state).
	 */
	TUICallbackRender	mRenderCallback;

	CUIDialog* mNextDlg;
	CUIDialog* mPrevDlg;

	int		mDefaultCtrlID;

	static double sTimeRefresh;
	double	mTimeLastRefresh;
	CUIControl* mCtrlMouseOver; // The control which is hovered over
};


// --------------------------------------------------------------------------

struct SUIFontNode {
	char	facename[MAX_PATH];
	ID3DXFont* font;
	LONG  height;
	LONG  weight;
};


//  Manages shared resources of dialogs.
class CUIResourceManager : public IDeviceResource {
public:
	static void initialize( int screenX, int screenY ) { assert(!mSingleInstance); mSingleInstance = new CUIResourceManager(screenX,screenY); }
	static void finalize() { delete mSingleInstance; }
	static CUIResourceManager& getInstance() { assert(mSingleInstance); return *mSingleInstance; }
public:
	int addFont( const char* facename, LONG height, LONG weight );
	SUIFontNode* getFontNode( int index ) { return mFontCache[ index ]; }

	float	xToBB( float x, int bbx ) const { return x * mInvScreenX * bbx; }
	float	yToBB( float y, int bby ) const { return y * mInvScreenY * bby; }
	int		bbToX( int bb, int bbx ) const { return bb * mScreenX / bbx; }
	int		bbToY( int bb, int bby ) const { return bb * mScreenY / bby; }

	// IDeviceResource
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

	// Shared between all dialogs
	IDirect3DStateBlock9* mStateBlock;
	ID3DXSprite*		  mSprite; // Sprite used for drawing

protected:
	static CUIResourceManager*	mSingleInstance;
	CUIResourceManager( int screenX, int screenY );
	~CUIResourceManager();
	HRESULT createFont( UINT index );

private:
	std::vector< SUIFontNode* > mFontCache; // Shared fonts
	int		mScreenX;
	int		mScreenY;
	float	mInvScreenX;
	float	mInvScreenY;
};


// --------------------------------------------------------------------------

// Base class for controls
class CUIControl : public boost::noncopyable {
public:
	CUIControl( CUIDialog *dialog = NULL );
	virtual ~CUIControl();

	//
	// Overridables

	virtual HRESULT onInit() { return S_OK; }
	virtual void refresh();
	virtual void render( IDirect3DDevice9* device, float dt ) { };

	virtual bool msgProc( UINT msg, WPARAM wParam, LPARAM lParam ) { return false; }

	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam ) { return false; }
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam ) { return false; }

	virtual bool canHaveFocus() const { return false; }
	virtual void onFocusIn() { mHasFocus = true; }
	virtual void onFocusOut() { mHasFocus = false; }
	virtual void onMouseEnter() { mMouseOver = true; }
	virtual void onMouseLeave() { mMouseOver = false; }
	virtual void onHotkey() {}

	virtual bool containsPoint( const POINT& pt ) const { return mBBox.containsPoint( pt ); }

	virtual void setEnabled( bool enabled ) { mEnabled = enabled; }
	virtual bool isEnabled() const { return mEnabled; }
	virtual void setVisible( bool vis ) { mVisible = vis; }
	virtual bool isVisible() const { return mVisible; }

	virtual void setTextColor( D3DCOLOR color );

	//
	// Getters/setters

	eUIControlType getType() const { return mType; }

	int  getID() const { return mID; }
	void setID( int cid ) { mID = cid; }

	void setLocation( float x, float y ) { mX = x; mY = y; updateRects(); }
	void setSize( float width, float height ) { mWidth = width; mHeight = height; updateRects(); }

	void setHotkey( UINT hotkey ) { mHotkey = hotkey; }
	UINT getHotkey() const { return mHotkey; }

	void setUserData( void *userdata ) { mUserData = userdata; }
	void *getUserData() const { return mUserData; }

	SUIElement* getElement( UINT index ) { return mElements[ index ]; }
	HRESULT setElement( UINT index, SUIElement* element );

	//
	// Public members

	bool mVisible;				// Shown/hidden flag
	bool mMouseOver;				// Mouse pointer is above control
	bool mHasFocus;				// Control has input focus
	bool mDefault;				// Is the default control

	// Size, scale, and positioning members
	float mX, mY;
	float mWidth, mHeight;

	// These members are set by the container
	CUIDialog*	mDialog;	// Parent container
	UINT		mIndex;		// Index within the control list
	
	std::vector< SUIElement* > mElements;  // Display elements

protected:
	virtual void	updateRects();

	SFRect mBBox; // Rectangle defining the active region of the control

	int				mID;		// Control ID number
	eUIControlType	mType;		// Control type, set once in constructor  
	UINT			mHotkey;	// Virtual key code for this control's hotkey
	void*			mUserData;	// Data associated with this control that is set by user.
	bool			mEnabled;	// enabled/disabled flag
};


// --------------------------------------------------------------------------

/// Static control
class CUIStatic : public CUIControl {
public:
	CUIStatic( CUIDialog *dialog = NULL );

	// CUIControl
	virtual void render( IDirect3DDevice9* device, float dt );
	virtual bool containsPoint( const POINT& pt ) const { return false; }

	HRESULT getTextCopy( char* strDest, UINT bufferCount );
	const char* getText() const { return mText; }
	HRESULT setText( const char* text );

protected:
	char mText[MAX_PATH];	   // Window text  
};


// --------------------------------------------------------------------------

/// Image control
class CUIImage : public CUIControl {
public:
	CUIImage( CUIDialog *dialog = NULL );

	void setTexture( CD3DTexture& tex, int u0, int v0, int u1, int v1 ) {
		RECT rc;
		rc.left = u0;	rc.right = u1;
		rc.top = v0;	rc.bottom = v1;
		getElement(0)->setTexture( &tex, &rc );
	}

	// CUIControl
	virtual void render( IDirect3DDevice9* device, float dt );
	virtual bool containsPoint( const POINT& pt ) const { return false; }
};


// --------------------------------------------------------------------------

/// Button control
class CUIButton : public CUIStatic {
public:
	CUIButton( CUIDialog *dialog = NULL );

	// CUIControl
	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual void onHotkey() { mDialog->sendEvent( UIEVENT_BUTTON_CLICKED, true, this ); }
	virtual bool containsPoint( const POINT& pt ) const { return mBBox.containsPoint( pt ); }
	virtual bool canHaveFocus() const { return (mVisible && mEnabled); }
	virtual void render( IDirect3DDevice9* device, float dt );

	bool	isPressed() const { return mPressed; }

protected:
	eUIControlState getState() const;
	
	bool	mPressed;
};


// --------------------------------------------------------------------------

/// Check box control
class CUICheckBox : public CUIButton {
public:
	CUICheckBox( CUIDialog *dialog = NULL );

	// CUIControl
	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual void onHotkey() { setCheckedInternal( !mChecked, true ); }
	virtual bool containsPoint( const POINT& pt ) const;
	virtual void updateRects(); 
	virtual void render( IDirect3DDevice9* device, float dt );

	bool isChecked() const { return mChecked; }
	void setChecked( bool chk ) { setCheckedInternal( chk, false ); }
	
protected:
	virtual void setCheckedInternal( bool chk, bool fromInput );

	bool mChecked;
	SFRect mRectButton;
	SFRect mRectText;
};

// --------------------------------------------------------------------------

/// Roll-out control
class CUIRollout : public CUICheckBox {
public:
	CUIRollout( CUIDialog *dialog = NULL );

	// CUIControl
	//virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	//virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	//virtual void onHotkey() { setCheckedInternal( !mChecked, true ); }
	virtual bool containsPoint( const POINT& pt ) const;
	virtual void updateRects(); 
	virtual void render( IDirect3DDevice9* device, float dt );

	bool	isExpanded() const { return isChecked(); }
	void	setExpanded( bool expanded ) { setChecked(expanded); }

	float	getRolloutHeight() const { return mRolloutHeight; }
	void	setRolloutHeight( float rh ) { mRolloutHeight = rh; }

	void	addChildControl( CUIControl& ctrl ) { mChildControls.push_back( &ctrl ); }

	/**
	 *	Offset location of the rollout and all child controls.
	 */
	void	offsetPos( float dx, float dy );
	
protected:
	virtual void setCheckedInternal( bool chk, bool fromInput );

	float	mRolloutHeight;

	std::vector<CUIControl*>	mChildControls;
};


// --------------------------------------------------------------------------

/// Radio button control
class CUIRadioButton : public CUICheckBox {
public:
	CUIRadioButton( CUIDialog *dialog = NULL );

	// CUIControl
	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual void onHotkey() { setCheckedInternal( true, true, true ); }
	
	void setChecked( bool chk, bool clearGroup = true ) { setCheckedInternal( chk, clearGroup, false ); }
	void setButtonGroup( int group ) { mButtonGroup = group; }
	int getButtonGroup() const { return mButtonGroup; }
	
protected:
	virtual void setCheckedInternal( bool chk, bool clearGroup, bool fromInput );
	int	mButtonGroup;
};


// --------------------------------------------------------------------------

/// Scrollbar control
class CUIScrollBar : public CUIControl {
public:
	CUIScrollBar( CUIDialog *dialog = NULL );
	virtual ~CUIScrollBar();

	// CUIControl
	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual void render( IDirect3DDevice9* device, float dt );
	virtual void updateRects();

	void	setTrackRange( int start, int end );
	int		getTrackPos() const { return mPosition; }
	void	setTrackPos( int pos ) { mPosition = pos; capPosition(); updateThumbRect(); }
	int		getPageSize() const { return mPageSize; }
	void	setPageSize( int pageSize ) { mPageSize = pageSize; capPosition(); updateThumbRect(); }

	void	scroll( int delta );	  // scroll by delta items (plus or minus)
	void	showItem( int index );  // Ensure that item index is displayed, scroll if necessary

protected:
	/// Indicates the state of the arrow buttons.
	enum eArrowState {
		CLEAR,			///< No arrow is down
		CLICKED_UP,		///< Up arrow clicked
		CLICKED_DOWN,	///< Down arrow clicked
		HELD_UP,		///< Up arrow is held down for sustained period
		HELD_DOWN		///< Down arrow is held down for sustained period
	};

	void	updateThumbRect();
	void	capPosition();  ///< Clips position at boundaries. Ensures it stays within legal range.

	bool	mShowThumb;
	SFRect	mRectUp;
	SFRect	mRectDown;
	SFRect	mRectTrack;
	SFRect	mRectThumb;
	int		mPosition;		// Position of the first displayed item
	int		mPageSize;		// How many items are displayable in one page
	int		mStart;			// First item
	int		mEnd;			// The index after the last item
	POINT	mLastMouse;		// Last mouse position
	eArrowState mArrows;	// State of the arrows
	double	mLastArrowTime;	// Timestamp of last arrow event.
};


// --------------------------------------------------------------------------

struct SUIListItem {
	char	text[256];
	const void*	data;

	SFRect	activeRect;
	bool	selected;
};

/// List box control
class CUIListBox : public CUIControl {
public:
	enum eStyle {
		MULTISELECTION = 1,
	};

public:
	CUIListBox( CUIDialog *dialog = NULL );
	virtual ~CUIListBox();

	// CUIControl
	virtual HRESULT onInit() { return mDialog->initControl( &mScrollBar ); }
	virtual bool	canHaveFocus() const { return (mVisible && mEnabled); }
	virtual bool	handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool	handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );

	virtual void	render( IDirect3DDevice9* device, float dt );
	virtual void	updateRects();

	DWORD	getStyle() const { return mStyle; }
	int		getSize() const { return mItems.size(); }
	void	setStyle( DWORD style ) { mStyle = style; }
	void	setScrollBarWidth( float width ) { mSBWidth = width; updateRects(); }
	void	setBorder( float border, float margin ) { mBorder = border; mMargin = margin; }
	HRESULT	addItem( const char *text, const void *data );
	HRESULT	insertItem( int index, const char *text, const void *data );
	void	removeItem( int index );
	void	removeItemByText( const char *text );
	void	removeItemByData( const void *data );
	void	removeAllItems();

	UINT	getItemCount() const { return mItems.size(); }
	SUIListItem* getItem( int index );
	int getSelectedIndex( int prevSelected = -1 ) const;
	SUIListItem* getSelectedItem( int prevSelected = -1 ) { return getItem( getSelectedIndex( prevSelected ) ); }
	void selectItem( int newIndex );

protected:
	CUIScrollBar mScrollBar;
	SFRect	mRectText;		// Text rendering bound
	SFRect	mRectSelection; // Selection box bound
	float	mSBWidth;
	float	mBorder;
	float	mMargin;
	float	mTextHeight;// Height of a single line of text
	DWORD	mStyle;		// List box style
	int		mSelected;	// Index of the selected item for single selection list box
	int		mSelStart;	// Index of the item where selection starts (for handling multi-selection)
	bool	mDragging;	// Whether the user is dragging the mouse to select

	std::vector< SUIListItem* > mItems;
};


// --------------------------------------------------------------------------

struct SUIComboItem {
	char	text[256];
	const void*	data;

	SFRect	activeRect;
	bool	vis;
};

/// ComboBox control
class CUIComboBox : public CUIButton {
public:
	CUIComboBox( CUIDialog *dialog = NULL );
	virtual ~CUIComboBox();

	// CUIControl
	virtual void setTextColor( D3DCOLOR color );
	virtual HRESULT onInit() { return mDialog->initControl( &mScrollBar ); }
	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual void onHotkey();
	virtual bool canHaveFocus() const { return (mVisible && mEnabled); }
	virtual void onFocusOut();
	virtual void render( IDirect3DDevice9* device, float dt );
	virtual void updateRects(); 

	HRESULT addItem( const char* text, const void* data, bool updateSelectionIf1st );
	void	removeAllItems();
	void	removeItem( UINT index );
	bool	containsItem( const char* text, UINT start=0 ) const;
	int 	findItem( const char* text, UINT start=0 ) const;
	const void*	getItemData( const char* text );
	void	setDropHeight( float height ) { mDropHeight = height; updateRects(); }
	void	setScrollBarWidth( float width ) { mSBWidth = width; updateRects(); }

	const void*	getSelectedData();
	SUIComboItem* getSelectedItem();

	UINT	getItemCount() const { return mItems.size(); }
	SUIComboItem* getItem( UINT index ) { return mItems[ index ]; }

	HRESULT setSelectedByIndex( UINT index );
	HRESULT setSelectedByText( const char* text );
	HRESULT setSelectedByData( const void* data );  

protected:
	CUIScrollBar mScrollBar;
	SFRect	mRectText;
	SFRect	mRectButton;
	SFRect	mRectDropdown;
	SFRect	mRectDropdownText;

	int		mSelected;
	int		mFocused;
	float	mDropHeight;
	float 	mSBWidth;

	bool	mOpened;
	
	std::vector< SUIComboItem* > mItems;
};


// --------------------------------------------------------------------------

/// Slider control
class CUISlider : public CUIControl {
public:
	CUISlider( CUIDialog *dialog = NULL );

	// CUIControl
	virtual bool containsPoint( const POINT& pt ) const;
	virtual bool canHaveFocus() const { return (mVisible && mEnabled); }
	virtual bool handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam );
	virtual bool handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual void updateRects(); 
	virtual void render( IDirect3DDevice9* device, float dt );

	void setValue( int value ) { setValueInternal( value, false ); }
	int  getValue() const { return mValue; };

	void setRange( int vmin, int vmax );

protected:
	void setValueInternal( int value, bool fromInput );
	int  getValueFromPos( int x ); 

	SFRect	mRectButton;

	int		mValue;

	int		mMin;
	int		mMax;

	int		mDragStartX;	// Mouse position at start of drag
	int		mDragOffset;	// Drag offset from the center of the button
	float	mButtonX;

	bool	mPressed;
};



// --------------------------------------------------------------------------
//  INLINE FUNCTIONS

inline bool CUIDialog::hasFocusControl() const
{
	return sCtrlFocus && sCtrlFocus->mDialog == this && sCtrlFocus->isEnabled();
}


}; // namespace dingus



#endif // DXUT_GUI_H
