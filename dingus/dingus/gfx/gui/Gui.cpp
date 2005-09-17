// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "Gui.h"
#include "../../resource/TextureBundle.h"



using namespace dingus;

// --------------------------------------------------------------------------
//  Constants

// Some things aren't always defined...
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020C
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif



#define V(x)		   { hr = x; }
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }



// Minimum scrollbar thumb size
const int SCROLLBAR_MINTHUMBSIZE = 8;
// Maximum scrollbar thumb size
const int SCROLLBAR_MAXTHUMBSIZE = 32;

// Delay and repeat period when clicking on the scroll bar arrows
const float SCROLLBAR_ARROWCLICK_DELAY = 0.33f;
const float SCROLLBAR_ARROWCLICK_REPEAT = 0.05f;



// --------------------------------------------------------------------------
//  Global/Static Members


CUIResourceManager* CUIResourceManager::mSingleInstance = 0;


double		CUIDialog::sTimeRefresh = 0.0f;
CUIControl*	CUIDialog::sCtrlFocus = NULL;	// The control which has focus
CUIControl*	CUIDialog::sCtrlPressed = NULL;	// The control currently pressed


struct SUIScreenVertex {
	float x, y, z, h;
	D3DCOLOR color;
	float tu, tv;
	static DWORD FVF;
};
DWORD SUIScreenVertex::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;


static inline int rectWidth( const RECT &rc ) { return ( (rc).right - (rc).left ); }
static inline int rectHeight( const RECT &rc ) { return ( (rc).bottom - (rc).top ); }


// --------------------------------------------------------------------------
//  CUIDialog class
// --------------------------------------------------------------------------

CUIDialog::CUIDialog()
:	mHasCaption(false), mMinimized(false),
	mX(0), mY(0), mWidth(0), mHeight(0), mCaptionHeight(18),
	mColorUL(0), mColorUR(0), mColorDL(0), mColorDR(0),
	mCallbackEvent(NULL), mDefaultTexture(NULL),
	mRenderCallback(NULL),
	mDefaultCtrlID(0xFFFF), mTimeLastRefresh(0),
	mCtrlMouseOver(NULL)
{
	mCaption[0] = 0;

	mNextDlg = this;
	mPrevDlg = this;

	mDoNonUserEvents = false;
	mDoKeyboardInput = false;
	mDoMouseInput = true;

	initDefaultElements();
}


CUIDialog::~CUIDialog()
{
	removeAllControls();

	mFonts.clear();

	for( int i = 0; i < mDefaultElements.size(); ++i ) {
		safeDelete( mDefaultElements[i] );
	}
	mDefaultElements.clear();
}


void CUIDialog::removeControl( int cid )
{
	int n = mControls.size();
	for( int i = 0; i < n; ++i ) {
		CUIControl* ctrl = mControls[ i ];
		if( ctrl->getID() == cid ) {
			// Clear focus first
			clearFocus();

			// clear references to this control
			if( sCtrlFocus == ctrl )		sCtrlFocus = NULL;
			if( sCtrlPressed == ctrl )		sCtrlPressed = NULL;
			if( mCtrlMouseOver == ctrl )	mCtrlMouseOver = NULL;

			safeDelete( ctrl );
			mControls.erase( mControls.begin() + i );
			return;
		}
	}
}


void CUIDialog::removeAllControls()
{
	if( sCtrlFocus && sCtrlFocus->mDialog == this )
		sCtrlFocus = NULL;
	if( sCtrlPressed && sCtrlPressed->mDialog == this )
		sCtrlPressed = NULL;
	mCtrlMouseOver = NULL;

	for( int i = 0; i < mControls.size(); ++i )
		safeDelete( mControls[i] );
	mControls.clear();
}


void CUIDialog::refresh()
{
	if( sCtrlFocus )
		sCtrlFocus->onFocusOut();

	if( mCtrlMouseOver )
		mCtrlMouseOver->onMouseLeave();

	sCtrlFocus = NULL;
	sCtrlPressed = NULL;
	mCtrlMouseOver = NULL;

	int n = mControls.size();
	for( int i=0; i < n; ++i ) {
		mControls[i]->refresh();
	}

	if( mDoKeyboardInput )
		focusDefaultControl();
}


void CUIDialog::onRender( float dt )
{	
	// See if the dialog needs to be refreshed
	if( mTimeLastRefresh < sTimeRefresh ) {
		mTimeLastRefresh = CSystemTimer::getInstance().getTimeS();
		refresh();
	}

	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();

	IDirect3DDevice9* device = &dx.getDevice();

	// set up a state block here and restore it when finished drawing all the controls
	resmgr.mStateBlock->Capture();

	device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	device->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

	device->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	device->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	device->SetVertexShader( NULL );
	device->SetPixelShader( NULL );

	device->SetRenderState( D3DRS_ZENABLE, FALSE );

	if( !mMinimized ) {
		device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
		device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

		float x1 = resmgr.xToBB( mX,			dx.getBackBufferWidth() ) - 0.5f;
		float x2 = resmgr.xToBB( mX+mWidth,		dx.getBackBufferWidth() ) - 0.5f;
		float y1 = resmgr.yToBB( mY,			dx.getBackBufferHeight() ) - 0.5f;
		float y2 = resmgr.yToBB( mY+mHeight,	dx.getBackBufferHeight() ) - 0.5f;

		SUIScreenVertex vertices[4] = {
			x1, y1, 0.5f, 1.0f, mColorUL, 0.0f, 0.5f,
			x1, y2, 0.5f, 1.0f, mColorDL, 1.0f, 0.5f,
			x2, y2, 0.5f, 1.0f, mColorDR, 1.0f, 1.0f,
			x2, y1, 0.5f, 1.0f, mColorUR, 0.0f, 1.0f,
		};
		device->SetFVF( SUIScreenVertex::FVF );
		device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SUIScreenVertex) );
	}

	device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	device->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	assert( mDefaultTexture );
	device->SetTexture( 0, mDefaultTexture->getObject() );

	CUIResourceManager::getInstance().mSprite->Begin( D3DXSPRITE_DONOTSAVESTATE );

	// call render callback
	if( mRenderCallback ) {
		mRenderCallback( *this );
	}

	// render the caption if it's enabled.
	if( mHasCaption ) {
		// drawSprite will offset the rect down by mCaptionHeight, so
		// adjust the rect higher here to negate the effect.
		SFRect rc = { 0, -mCaptionHeight, mWidth, 0 };
		drawSprite( &mCaptionElement, &rc );
		rc.left += 5; // Make a left margin
		char output[256];
		strncpy( output, mCaption, 256 );
		output[255] = 0;
		if( mMinimized )
			strncat( output, " (Minimized)", 256 - strlen( output ) );
		drawText( output, &mCaptionElement, &rc, !mCaptionElement.darkFont );
	}

	// If the dialog is minimized, skip rendering its controls.
	if( !mMinimized ) {
		int n = mControls.size();
		for( int i = 0; i < n; ++i ) {
			CUIControl* ctrl = mControls[i];
			// Focused control is drawn last
			if( ctrl == sCtrlFocus )
				continue;
			ctrl->render( device, dt );
		}
		if( sCtrlFocus != NULL && sCtrlFocus->mDialog == this )
			sCtrlFocus->render( device, dt );
	}

	//RECT rc;
	//SetRect( &rc, 0, 0, mWidth, mHeight );
	//drawRect( &rc, 0x80ffffff );
	
	CUIResourceManager::getInstance().mSprite->End();
	CUIResourceManager::getInstance().mStateBlock->Apply();
}


void CUIDialog::sendEvent( UINT evt, bool userTriggered, CUIControl* ctrl )
{
	// If no callback has been registered there's nowhere to send the event to
	if( mCallbackEvent == NULL )
		return;
	// Discard events triggered programatically if these types of events
	// haven't been enabled
	if( !userTriggered && !mDoNonUserEvents )
		return;
	mCallbackEvent( evt, ctrl->getID(), ctrl );
}


void CUIDialog::setFont( UINT index, const char* facename, LONG height, LONG weight )
{
	// Make sure the list is at least as large as the index being set
	for( UINT i = mFonts.size(); i <= index; ++i )
		mFonts.push_back( -1 );

	int fontIdx = CUIResourceManager::getInstance().addFont( facename, height, weight );
	mFonts[index] = fontIdx;
}


SUIFontNode* CUIDialog::getFont( UINT index ) const
{
	assert( index >= 0 && index < mFonts.size() );
	return CUIResourceManager::getInstance().getFontNode( mFonts[ index ] );
}



bool CUIDialog::msgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	bool handled = false;

	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& uiresmgr = CUIResourceManager::getInstance();
	POINT mousePoint;
	mousePoint.x = uiresmgr.bbToX( short(LOWORD(lParam)), dx.getBackBufferWidth() );
	mousePoint.y = uiresmgr.bbToY( short(HIWORD(lParam)), dx.getBackBufferHeight() );

	// If caption is enabled, check for clicks in the caption area.
	if( mHasCaption ) {
		static bool dragging;

		if( msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK ) {
			if( mousePoint.x >= mX && mousePoint.x < mX + mWidth &&
				mousePoint.y >= mY && mousePoint.y < mY + mCaptionHeight )
			{
				dragging = true;
				SetCapture( CD3DDevice::getInstance().getDeviceWindow() );
				return true;
			}
		} else if( msg == WM_LBUTTONUP && dragging ) {
			if( mousePoint.x >= mX && mousePoint.x < mX + mWidth &&
				mousePoint.y >= mY && mousePoint.y < mY + mCaptionHeight )
			{
				ReleaseCapture();
				dragging = false;
				mMinimized = !mMinimized;
				return true;
			}
		}
	}

	// If the dialog is minimized, don't send any messages to controls.
	if( mMinimized )
		return false;

	// Give the first chance at handling the message to the focused control.
	bool isFocus = hasFocusControl();
	if( isFocus ) {
		// If the control handles it, then we're done.
		if( sCtrlFocus->msgProc( msg, wParam, lParam ) )
			return true;
	}

	switch( msg ) {
		case WM_ACTIVATEAPP:
			// Call onFocusIn()/onFocusOut() of the control that currently
			// has the focus as the application is activated/deactivated.
			// This matches the Windows behavior.
			if( isFocus ) {
				if( wParam )
					sCtrlFocus->onFocusIn();
				else
					sCtrlFocus->onFocusOut();
			}
			break;

		// Keyboard messages
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			// Give first chance to the focused control.
			if( isFocus ) {
				if( sCtrlFocus->handleKeyb( msg, wParam, lParam ) )
					return true;
			}

			// Not yet handled, see if this matches a control's hotkey
			// [Activate the hotkey if the focus doesn't belong to an edit box.]
			if( msg == WM_KEYUP ) {
				int n = mControls.size();
				for( int i = 0; i < n; ++i ) {
					CUIControl* ctrl = mControls[ i ];
					if( ctrl->isEnabled() && ctrl->getHotkey() == wParam ) {
						ctrl->onHotkey();
						return true;
					}
				}
			}

			// Not yet handled, check for focus messages
			if( msg == WM_KEYDOWN ) {
				// If keyboard input is not enabled, this message should be ignored
				if( !mDoKeyboardInput )
					return false;

				switch( wParam ) {
					case VK_RIGHT:
					case VK_DOWN:
						if( sCtrlFocus != NULL ) {
							onCycleFocus( true );
							return true;
						}
						break;
					case VK_LEFT:
					case VK_UP:
						if( sCtrlFocus != NULL ) {
							onCycleFocus( false );
							return true;
						}
						break;
					case VK_TAB: 
						if( sCtrlFocus == NULL ) {
							focusDefaultControl();
						} else {
							bool shiftDown = ((GetAsyncKeyState( VK_SHIFT ) & 0x8000) != 0);
							onCycleFocus( !shiftDown );
						}
						return true;
				}
			}
			break;
		}

		// Mouse messages
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		//case WM_XBUTTONDBLCLK: // TBD
		case WM_MOUSEWHEEL:
		{
			// If not accepting mouse input, return false to indicate the
			// message should still be handled by the application
			// (usually to move the camera).
			if( !mDoMouseInput )
				return false;

			mousePoint.x -= mX;
			mousePoint.y -= mY;

			// If caption is enabled, offset the Y coordinate by the
			// negative of its height.
			if( mHasCaption )
				mousePoint.y -= mCaptionHeight;

			// Give first chance to the focused control.
			if( isFocus ) {
				if( sCtrlFocus->handleMouse( msg, mousePoint, wParam, lParam ) )
					return true;
			}

			// Not yet handled, see if the mouse is over any controls
			CUIControl* ctrl = getControlAtPoint( mousePoint );
			if( ctrl != NULL && ctrl->isEnabled() ) {
				handled = ctrl->handleMouse( msg, mousePoint, wParam, lParam );
				if( handled )
					return true;
			} else {
				// Mouse not over any controls in this dialog, if there was
				// a control which had focus it just lost it
				if( msg == WM_LBUTTONDOWN && sCtrlFocus && sCtrlFocus->mDialog == this ) {
					sCtrlFocus->onFocusOut();
					sCtrlFocus = NULL;
				}
			}

			// Still not handled, hand this off to the dialog. Return false
			// to indicate the message should still be handled by the
			// application (usually to move the camera).
			switch( msg ) {
				case WM_MOUSEMOVE:
					onMouseMove( mousePoint );
					return false;
			}
			break;
		}
	}

	// Not handled
	return false;
}

CUIControl* CUIDialog::getControlAtPoint( POINT pt )
{
	// Search through all child controls for the first one which
	// contains the mouse point
	int n = mControls.size();
	for( int i=0; i < n; ++i ) {
		CUIControl* ctrl = mControls[i];
		assert( ctrl );
		//if( ctrl == NULL )
		//	continue;

		// We only return the current control if it is visible
		// and enabled.  Because getControlAtPoint() is used to do mouse
		// hittest, it makes sense to perform this filtering.
		if( ctrl->containsPoint( pt ) && ctrl->isEnabled() && ctrl->isVisible() )
			return ctrl;
	}

	return NULL;
}


bool CUIDialog::isControlEnabled( int cid )
{
	CUIControl* ctrl = getControl( cid );
	if( ctrl == NULL )
		return false;
	return ctrl->isEnabled();
}


void CUIDialog::setControlEnabled( int cid, bool enabled )
{
	CUIControl* ctrl = getControl( cid );
	if( ctrl == NULL )
		return;
	ctrl->setEnabled( enabled );
}


void CUIDialog::onMouseUp( POINT pt )
{
	sCtrlPressed = NULL;
	mCtrlMouseOver = NULL;
}

void CUIDialog::onMouseMove( POINT pt )
{
	// Figure out which control the mouse is over now
	CUIControl* ctrl = getControlAtPoint( pt );

	// If the mouse is still over the same control, nothing needs to be done
	if( ctrl == mCtrlMouseOver )
		return;

	// Handle mouse leaving the old control
	if( mCtrlMouseOver )
		mCtrlMouseOver->onMouseLeave();

	// Handle mouse entering the new control
	mCtrlMouseOver = ctrl;
	if( ctrl != NULL )
		mCtrlMouseOver->onMouseEnter();
}


HRESULT CUIDialog::setDefaultElement( eUIControlType ctrlType, UINT index, SUIElement* element )
{
	// If this element type already exist in the list, simply update the stored element
	int n = mDefaultElements.size();
	for( int i = 0; i < n; ++i ) {
		SElementHolder* holder = mDefaultElements[ i ];
		
		if( holder->ctrlType == ctrlType && holder->index == index ) {
			holder->element = *element;
			return S_OK;
		}
	}

	// Otherwise, add a new entry
	SElementHolder* newHolder;
	newHolder = new SElementHolder;
	if( newHolder == NULL )
		return E_OUTOFMEMORY;

	newHolder->ctrlType = ctrlType;
	newHolder->index = index;
	newHolder->element = *element;

	mDefaultElements.push_back( newHolder );
	return S_OK;
}


SUIElement* CUIDialog::getDefaultElement( eUIControlType ctrlType, UINT index )
{
	int n = mDefaultElements.size();
	for( int i = 0; i < n; ++i ) {
		SElementHolder* holder = mDefaultElements[ i ];
		if( holder->ctrlType == ctrlType && holder->index == index )
			return &holder->element;
	}
	return NULL;
}



// --------------------------------------------------------------------------


HRESULT CUIDialog::addStatic( int cid, const char* text, int x, int y, int width, int height, bool isDefault, CUIStatic** created )
{
	HRESULT hr = S_OK;

	CUIStatic* ctrl = new CUIStatic( this );
	if( created != NULL )
		*created = ctrl;
	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setText( text );
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->mDefault = isDefault;

	return S_OK;
}

HRESULT CUIDialog::addImage( int cid, int x, int y, int width, int height, CD3DTexture& tex, int u0, int v0, int u1, int v1, CUIImage** created )
{
	HRESULT hr = S_OK;

	CUIImage* ctrl = new CUIImage( this );
	if( created != NULL )
		*created = ctrl;
	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	ctrl->setTexture( tex, u0, v0, u1, v1 );

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->mDefault = false;

	return S_OK;
}


HRESULT CUIDialog::addButton( int cid, const char* text, int x, int y, int width, int height, UINT hotkey, bool isDefault, CUIButton** created )
{
	HRESULT hr = S_OK;

	CUIButton* ctrl = new CUIButton( this );

	if( created != NULL )
		*created = ctrl;

	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setText( text );
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->setHotkey( hotkey );
	ctrl->mDefault = isDefault;

	return S_OK;
}


HRESULT CUIDialog::addCheckBox( int cid, const char* text, int x, int y, int width, int height, bool chk, UINT hotkey, bool isDefault, CUICheckBox** created )
{
	HRESULT hr = S_OK;

	CUICheckBox* ctrl = new CUICheckBox( this );

	if( created != NULL )
		*created = ctrl;

	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setText( text );
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->setHotkey( hotkey );
	ctrl->mDefault = isDefault;
	ctrl->setChecked( chk );
	
	return S_OK;
}

HRESULT CUIDialog::addRollout( int cid, const char* text, int x, int y, int width, int height, int rollHeight, bool expanded, UINT hotkey, bool isDefault, CUIRollout** created )
{
	HRESULT hr = S_OK;

	CUIRollout* ctrl = new CUIRollout( this );
	if( created != NULL )
		*created = ctrl;
	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	ctrl->setID( cid ); 
	ctrl->setText( text );
	ctrl->setRolloutHeight( rollHeight );
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->setHotkey( hotkey );
	ctrl->mDefault = isDefault;
	ctrl->setExpanded( expanded );
	
	return S_OK;
}


HRESULT CUIDialog::addRadioButton( int cid, UINT group, const char* text, int x, int y, int width, int height, bool chk, UINT hotkey, bool isDefault, CUIRadioButton** created )
{
	HRESULT hr = S_OK;

	CUIRadioButton* ctrl = new CUIRadioButton( this );

	if( created != NULL )
		*created = ctrl;

	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setText( text );
	ctrl->setButtonGroup( group );
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->setHotkey( hotkey );
	ctrl->setChecked( chk );
	ctrl->mDefault = isDefault;
	ctrl->setChecked( chk );

	return S_OK;
}


HRESULT CUIDialog::addComboBox( int cid, int x, int y, int width, int height, UINT hotkey, bool isDefault, CUIComboBox** created )
{
	HRESULT hr = S_OK;

	CUIComboBox* ctrl = new CUIComboBox( this );

	if( created != NULL )
		*created = ctrl;

	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->setHotkey( hotkey );
	ctrl->mDefault = isDefault;

	return S_OK;
}


HRESULT CUIDialog::addSlider( int cid, int x, int y, int width, int height, int min, int max, int value, bool isDefault, CUISlider** created )
{
	HRESULT hr = S_OK;

	CUISlider* ctrl = new CUISlider( this );

	if( created != NULL )
		*created = ctrl;

	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and list index
	ctrl->setID( cid ); 
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->mDefault = isDefault;
	ctrl->setRange( min, max );
	ctrl->setValue( value );
	ctrl->updateRects();

	return S_OK;
}


HRESULT CUIDialog::addListBox( int cid, int x, int y, int width, int height, DWORD style, CUIListBox** created )
{
	HRESULT hr = S_OK;
	CUIListBox *ctrl = new CUIListBox( this );

	if( created != NULL )
		*created = ctrl;

	if( ctrl == NULL )
		return E_OUTOFMEMORY;

	hr = addControl( ctrl );
	if( FAILED(hr) )
		return hr;

	// set the cid and position
	ctrl->setID( cid );
	ctrl->setLocation( x, y );
	ctrl->setSize( width, height );
	ctrl->setStyle( style );

	return S_OK;
}


HRESULT CUIDialog::initControl( CUIControl* ctrl )
{
	HRESULT hr;

	if( ctrl == NULL )
		return E_INVALIDARG;

	ctrl->mIndex = mControls.size();
	
	// Look for a default element entries
	for( int i=0; i < mDefaultElements.size(); i++ ) {
		SElementHolder* holder = mDefaultElements[ i ];
		if( holder->ctrlType == ctrl->getType() )
			ctrl->setElement( holder->index, &holder->element );
	}
	V_RETURN( ctrl->onInit() );
	return S_OK;
}


HRESULT CUIDialog::addControl( CUIControl* ctrl )
{
	HRESULT hr = S_OK;
	hr = initControl( ctrl );
	if( FAILED(hr) ) {
		ASSERT_FAIL_MSG( "CUIDialog::initControl" );
		return hr;
	}
	// add to the list
	mControls.push_back( ctrl );
	return S_OK;
}


CUIControl* CUIDialog::getControl( int cid )
{
	int n = mControls.size();
	for( int i = 0; i < n; ++i ) {
		CUIControl* ctrl = mControls[ i ];
		if( ctrl->getID() == cid )
			return ctrl;
	}
	return NULL;
}


CUIControl* CUIDialog::getControl( int cid, eUIControlType ctrlType )
{
	int n = mControls.size();
	for( int i = 0; i < n; ++i ) {
		CUIControl* ctrl = mControls[ i ];
		if( ctrl->getID() == cid && ctrl->getType() == ctrlType )
			return ctrl;
	}
	return NULL;
}


CUIControl* CUIDialog::getNextControl( CUIControl* ctrl )
{
	int index = ctrl->mIndex + 1;
	CUIDialog* dialog = ctrl->mDialog;
	// Cycle through dialogs in the loop to find the next control. Note
	// that if only one control exists in all looped dialogs it will
	// be the returned 'next' control.
	while( index >= (int)dialog->mControls.size() ) {
		dialog = dialog->mNextDlg;
		index = 0;
	}
	return dialog->mControls[ index ];
}

CUIControl* CUIDialog::getPrevControl( CUIControl* ctrl )
{
	int index = ctrl->mIndex - 1;
	CUIDialog* dialog = ctrl->mDialog;
	// Cycle through dialogs in the loop to find the next control. Note
	// that if only one control exists in all looped dialogs it will
	// be the returned 'previous' control.
	while( index < 0 ) {
		dialog = dialog->mPrevDlg;
		if( dialog == NULL )
			dialog = ctrl->mDialog;
		index = dialog->mControls.size() - 1;
	}
	return dialog->mControls[ index ];	
}


// --------------------------------------------------------------------------

void CUIDialog::clearRadioButtonGroup( UINT group )
{
	// Find all radio buttons with the given group number
	int n = mControls.size();
	for( int i = 0; i < n; ++i ) {
		CUIControl* ctrl = mControls[ i ];
		if( ctrl->getType() == UICTRL_RADIOBUTTON ) {
			CUIRadioButton* rctrl = (CUIRadioButton*) ctrl;
			if( rctrl->getButtonGroup() == group )
				rctrl->setChecked( false, false );
		}
	}
}


void CUIDialog::clearComboBox( int cid )
{
	CUIComboBox* ctrl = getComboBox( cid );
	if( ctrl == NULL )
		return;
	ctrl->removeAllItems();
}


void CUIDialog::requestFocus( CUIControl* ctrl )
{
	if( sCtrlFocus == ctrl )
		return;
	if( !ctrl->canHaveFocus() )
		return;
	if( sCtrlFocus )
		sCtrlFocus->onFocusOut();
	ctrl->onFocusIn();
	sCtrlFocus = ctrl;
}


// --------------------------------------------------------------------------

HRESULT CUIDialog::drawRect( const SFRect* rect, D3DCOLOR color )
{
	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();

	SFRect rcScreen = *rect;
	rcScreen.offset( mX, mY );

	// If caption is enabled, offset the Y position by its height.
	if( mHasCaption )
		rcScreen.offset( 0, mCaptionHeight );

	float x1 = resmgr.xToBB( rcScreen.left,   dx.getBackBufferWidth() ) - 0.5f;
	float x2 = resmgr.xToBB( rcScreen.right,  dx.getBackBufferWidth() ) - 0.5f;
	float y1 = resmgr.yToBB( rcScreen.top,    dx.getBackBufferHeight() ) - 0.5f;
	float y2 = resmgr.yToBB( rcScreen.bottom, dx.getBackBufferHeight() ) - 0.5f;

	SUIScreenVertex vertices[4] = {
		x1, y1, 0.5f, 1.0f, color, 0, 0,
		x2, y1, 0.5f, 1.0f, color, 0, 0,
		x2, y2, 0.5f, 1.0f, color, 0, 0,
		x1, y2, 0.5f, 1.0f, color, 0, 0,
	};

	IDirect3DDevice9* device = &dx.getDevice();

	// Since we're doing our own drawing here we need to flush the sprites
	resmgr.mSprite->Flush();
	IDirect3DVertexDeclaration9 *decl = NULL;
	device->GetVertexDeclaration( &decl );  // Preserve the sprite's current vertex decl
	device->SetFVF( SUIScreenVertex::FVF );

	device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
	device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );

	device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SUIScreenVertex) );

	device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );

	// Restore the vertex decl
	device->SetVertexDeclaration( decl );
	decl->Release();

	return S_OK;
}


/*
HRESULT CUIDialog::drawPolyLine( const POINT* points, UINT pointCount, D3DCOLOR color )
{
	SUIScreenVertex* vertices = new SUIScreenVertex[ pointCount ];
	if( vertices == NULL )
		return E_OUTOFMEMORY;

	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();

	SUIScreenVertex* vtx = vertices;
	const POINT* pt = points;
	for( UINT i = 0; i < pointCount; ++i ) {
		vtx->x = resmgr.xToBB( mX + pt->x, dx.getBackBufferWidth() );
		vtx->y = resmgr.yToBB( mY + pt->y, dx.getBackBufferHeight() );
		vtx->z = 0.5f;
		vtx->h = 1.0f;
		vtx->color = color;
		vtx->tu = 0.0f;
		vtx->tv = 0.0f;

		vtx++;
		pt++;
	}

	IDirect3DDevice9* device = &dx.getDevice();

	// Since we're doing our own drawing here we need to flush the sprites
	resmgr.mSprite->Flush();
	IDirect3DVertexDeclaration9 *decl = NULL;
	device->GetVertexDeclaration( &decl );  // Preserve the sprite's current vertex decl
	device->SetFVF( SUIScreenVertex::FVF );

	device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
	device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );

	device->DrawPrimitiveUP( D3DPT_LINESTRIP, pointCount - 1, vertices, sizeof(SUIScreenVertex) );

	device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );

	// Restore the vertex decl
	device->SetVertexDeclaration( decl );
	decl->Release();

	safeDeleteArray( vertices );
	return S_OK;
}
*/ 


HRESULT CUIDialog::drawSprite( SUIElement* element, const SFRect* dest )
{
	SFRect destScr = *dest;
	destScr.offset( mX, mY );
	// If caption is enabled, offset the Y position by its height.
	if( mHasCaption )
		destScr.offset( 0, mCaptionHeight );

	return imDrawSprite( element->colorTexture.current, element->textureRect, element->texture, destScr );
}


/*
HRESULT CUIDialog::calcTextRect( const char* text, SUIElement* element, SFRect* dest, int count )
{
	HRESULT hr = S_OK;

	SUIFontNode* fontNode = getFont( element->fontIdx );
	DWORD textFormat = element->textFormat | DT_CALCRECT;
	// Since we are only computing the rectangle, we don't need a sprite.
	hr = fontNode->font->DrawText( NULL, text, count, dest, textFormat, element->colorFont.current );
	if( FAILED(hr) )
		return hr;
	return S_OK;
}
*/

HRESULT CUIDialog::drawText( const char* text, SUIElement* element, const SFRect* dest, bool shadow, int count )
{
	SFRect destScr = *dest;
	destScr.offset( mX, mY );
	// If caption is enabled, offset the Y position by its height.
	if( mHasCaption )
		destScr.offset( 0, mCaptionHeight );

	return imDrawText( text, element->fontIdx, element->textFormat, element->colorFont.current, destScr, shadow, count );
}


HRESULT CUIDialog::drawText( const wchar_t* text, SUIElement* element, const SFRect* dest, bool shadow, int count )
{
	SFRect destScr = *dest;
	destScr.offset( mX, mY );
	// If caption is enabled, offset the Y position by its height.
	if( mHasCaption )
		destScr.offset( 0, mCaptionHeight );

	return imDrawText( text, element->fontIdx, element->textFormat, element->colorFont.current, destScr, shadow, count );
}


// --------------------------------------------------------------------------


HRESULT CUIDialog::imDrawSprite( const D3DXCOLOR& color, const RECT& texRect, CD3DTexture* tex, const SFRect& destScr )
{
	// No need to draw fully transparent layers
	if( color.a == 0 )
		return S_OK;

	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();
	
	float x1 = resmgr.xToBB( destScr.left,   dx.getBackBufferWidth() );
	float x2 = resmgr.xToBB( destScr.right,  dx.getBackBufferWidth() );
	float y1 = resmgr.yToBB( destScr.top,    dx.getBackBufferHeight() );
	float y2 = resmgr.yToBB( destScr.bottom, dx.getBackBufferHeight() );

	float scaleX = (x2-x1) / rectWidth( texRect );
	float scaleY = (y2-y1) / rectHeight( texRect );

	D3DXMATRIXA16 matrix;
	D3DXMatrixScaling( &matrix, scaleX, scaleY, 1.0f );

	resmgr.mSprite->SetTransform( &matrix );
	
	D3DXVECTOR3 pos( x1/scaleX, y1/scaleY, 0.0f );

	return resmgr.mSprite->Draw( tex->getObject(), &texRect, NULL, &pos, color );
}


HRESULT CUIDialog::imDrawText( const char* text, UINT fontIdx, DWORD format, const D3DXCOLOR& color, const SFRect& destScr, bool shadow, int count )
{
	HRESULT hr = S_OK;

	// No need to draw fully transparent layers
	if( color.a == 0 )
		return S_OK;

	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();

	SFRect rcScreen;
	rcScreen.left   = resmgr.xToBB( destScr.left,   dx.getBackBufferWidth() );
	rcScreen.right  = resmgr.xToBB( destScr.right,  dx.getBackBufferWidth() );
	rcScreen.top    = resmgr.yToBB( destScr.top,    dx.getBackBufferHeight() );
	rcScreen.bottom = resmgr.yToBB( destScr.bottom, dx.getBackBufferHeight() );

	D3DXMATRIXA16 matrix;
	D3DXMatrixIdentity( &matrix );
	resmgr.mSprite->SetTransform( &matrix );

	SUIFontNode* fontNode = getFont( fontIdx );

	RECT drawRC;
	rcScreen.toRect( drawRC );
	
	if( shadow ) {
		RECT rcShadow = drawRC;
		OffsetRect( &rcShadow, 1, 1 );
		hr = fontNode->font->DrawText( resmgr.mSprite, text, count, &rcShadow, format, D3DCOLOR_ARGB(DWORD(color.a * 255), 0, 0, 0) );
		if( FAILED(hr) )
			return hr;
	}

	hr = fontNode->font->DrawText( resmgr.mSprite, text, count, &drawRC, format, color );
	if( FAILED(hr) )
		return hr;

	return S_OK;
}


HRESULT CUIDialog::imDrawText( const wchar_t* text, UINT fontIdx, DWORD format, const D3DXCOLOR& color, const SFRect& destScr, bool shadow, int count )
{
	HRESULT hr = S_OK;

	// No need to draw fully transparent layers
	if( color.a == 0 )
		return S_OK;

	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();

	SFRect rcScreen;
	rcScreen.left   = resmgr.xToBB( destScr.left,   dx.getBackBufferWidth() );
	rcScreen.right  = resmgr.xToBB( destScr.right,  dx.getBackBufferWidth() );
	rcScreen.top    = resmgr.yToBB( destScr.top,    dx.getBackBufferHeight() );
	rcScreen.bottom = resmgr.yToBB( destScr.bottom, dx.getBackBufferHeight() );

	D3DXMATRIXA16 matrix;
	D3DXMatrixIdentity( &matrix );
	resmgr.mSprite->SetTransform( &matrix );

	SUIFontNode* fontNode = getFont( fontIdx );

	RECT drawRC;
	rcScreen.toRect( drawRC );
	
	if( shadow ) {
		RECT rcShadow = drawRC;
		OffsetRect( &rcShadow, 1, 1 );
		hr = fontNode->font->DrawTextW( resmgr.mSprite, text, count, &rcShadow, format, D3DCOLOR_ARGB(DWORD(color.a * 255), 0, 0, 0) );
		if( FAILED(hr) )
			return hr;
	}

	hr = fontNode->font->DrawTextW( resmgr.mSprite, text, count, &drawRC, format, color );
	if( FAILED(hr) )
		return hr;

	return S_OK;
}


// --------------------------------------------------------------------------

void CUIDialog::setBackgroundColors( D3DCOLOR colorUL, D3DCOLOR colorUR, D3DCOLOR colorDL, D3DCOLOR colorDR )
{
	mColorUL = colorUL;
	mColorUR = colorUR;
	mColorDL = colorDL;
	mColorDR = colorDR;
}


void CUIDialog::setNextDialog( CUIDialog* nextDlg )
{ 
	if( nextDlg == NULL )
		nextDlg = this;
	mNextDlg = nextDlg;
	mNextDlg->mPrevDlg = this;
}


void CUIDialog::clearFocus()
{
	if( sCtrlFocus ) {
		sCtrlFocus->onFocusOut();
		sCtrlFocus = NULL;
	}
}


void CUIDialog::focusDefaultControl()
{
	// Check for default control in this dialog
	int n = mControls.size();
	for( int i = 0; i < n; ++i ) {
		CUIControl* ctrl = mControls[ i ];
		if( ctrl->mDefault ) {
			// remove focus from the current control
			clearFocus();
			// Give focus to the default control
			sCtrlFocus = ctrl;
			sCtrlFocus->onFocusIn();
			return;
		}
	}
}


void CUIDialog::onCycleFocus( bool forward )
{
	// This should only be handled by the dialog which owns the focused
	// control, and only if a control currently has focus
	if( sCtrlFocus == NULL || sCtrlFocus->mDialog != this )
		return;

	CUIControl* ctrl = sCtrlFocus;
	for( int i=0; i < 0xFFFF; i++ ) {
		ctrl = (forward) ? getNextControl( ctrl ) : getPrevControl( ctrl );
		
		// If we've gone in a full circle then focus doesn't change
		if( ctrl == sCtrlFocus )
			return;

		// If the dialog accepts keybord input and the control can have focus then
		// move focus
		if( ctrl->mDialog->mDoKeyboardInput && ctrl->canHaveFocus() ) {
			sCtrlFocus->onFocusOut();
			sCtrlFocus = ctrl;
			sCtrlFocus->onFocusIn();
			return;
		}
	}

	// If we reached this point, the chain of dialogs didn't form a complete loop
	ASSERT_FAIL_MSG( "CUIDialog: Multiple dialogs are improperly chained together" );
}


void CUIDialog::onMouseEnter( CUIControl* ctrl )
{
	if( ctrl == NULL )
		return;
	// TBD - implementation?
	//ctrl->mMouseOver = true;
}


void CUIDialog::onMouseLeave( CUIControl* ctrl )
{
	if( ctrl == NULL )
		return;
	// TBD - implementation?
	//ctrl->mMouseOver = false;
}


// --------------------------------------------------------------------------

const D3DCOLOR	colINormal	= 0xC0ffffff;
const D3DCOLOR	colIPressed	= 0xF0e0e0d0;
const D3DCOLOR	colIHover	= 0xF0ffffe0;
const D3DCOLOR	colIFocus	= 0xE0ffffe0;
const D3DCOLOR	colIDisable	= 0x80808080;
const D3DCOLOR	colHidden = 0x00000000;

const D3DCOLOR	colTDNormal	= 0xE0000000;
const D3DCOLOR	colTDPress	= 0xF0202020;
const D3DCOLOR	colTDHover	= 0xFF202000;
const D3DCOLOR	colTDFocus	= 0xFF202000;
const D3DCOLOR	colTDDisable= 0x80404040;

const D3DCOLOR	colTLNormal	= 0xE0ffffff;
const D3DCOLOR	colTLPress	= 0xF0e0e0d0;
const D3DCOLOR	colTLHover	= 0xFFffffe0;
const D3DCOLOR	colTLFocus	= 0xFFffffe0;
const D3DCOLOR	colTLDisable= 0x80808080;



void CUIDialog::initDefaultElements()
{
	CD3DTexture* tex = RGET_TEX("guiskin");
	setDefaultTexture( *tex );
	setFont( 0, "Tahoma", 12, FW_NORMAL );
	
	SUIElement element;
	RECT textureRect;


	// -------------------------------------
	//  element for the caption

	SetRect( &textureRect, 17, 269, 241, 287 );
	mCaptionElement.setTexture( tex, &textureRect );
	//mCaptionElement.colorTexture.colors[ UISTATE_NORMAL ] = D3DCOLOR_ARGB(255, 255, 255, 255);
	mCaptionElement.setFont( 0, false, DT_LEFT | DT_VCENTER );
	// Pre-blend as we don't need to transition the state
	mCaptionElement.colorTexture.blend( UISTATE_NORMAL, 10.0f );
	mCaptionElement.colorFont.blend( UISTATE_NORMAL, 10.0f );


	// -------------------------------------
	//  CUIStatic

	element.setFont( 0, false, DT_LEFT | DT_VCENTER );
	element.colorFont.colors[ UISTATE_DISABLED ] = 0xA0a0a0a0;
	setDefaultElement( UICTRL_STATIC, 0, &element );
	
	// -------------------------------------
	//  CUIImage

	SetRect( &textureRect, 0, 0, 512, 512 );
	element.setTexture( tex, &textureRect );
	element.colorTexture.colors[ UISTATE_NORMAL ] = 0xFFffffff;
	setDefaultElement( UICTRL_IMAGE, 0, &element );

	
	// -------------------------------------
	//  CUIButton

	// Button
	SetRect( &textureRect, 0, 320, 136, 394 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, true );
	setDefaultElement( UICTRL_BUTTON, 0, &element );

	
	// -------------------------------------
	//  CUICheckBox

	// Box
	SetRect( &textureRect, 19, 394, 38, 413 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, false, DT_LEFT | DT_VCENTER );
	setDefaultElement( UICTRL_CHECKBOX, 0, &element );

	// Check
	SetRect( &textureRect, 0, 394, 19, 413 );
	element.setTexture( tex, &textureRect );
	setDefaultElement( UICTRL_CHECKBOX, 1, &element );

	// -------------------------------------
	//  CUIRollout

	// Box
	SetRect( &textureRect, 0, 24, 256, 49 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, true, DT_CENTER | DT_VCENTER );
	setDefaultElement( UICTRL_ROLLOUT, 0, &element );

	// Rollout
	SetRect( &textureRect, 0, 106, 256, 320 );
	element.setTexture( tex, &textureRect );
	element.colorTexture.colors[UISTATE_MOUSEOVER] = colIFocus;
	setDefaultElement( UICTRL_ROLLOUT, 1, &element );


	// -------------------------------------
	//  CUISlider

	// Track
	SetRect( &textureRect, 0, 0, 256, 24 );
	element.setTexture( tex, &textureRect );
	//element.colorTexture.colors[ UISTATE_NORMAL ] = D3DCOLOR_ARGB(150, 255, 255, 255);
	//element.colorTexture.colors[ UISTATE_FOCUS ] = D3DCOLOR_ARGB(200, 255, 255, 255);
	//element.colorTexture.colors[ UISTATE_DISABLED ] = D3DCOLOR_ARGB(70, 255, 255, 255);
	setDefaultElement( UICTRL_SLIDER, 0, &element );

	// Button
	SetRect( &textureRect, 135, 54, 162, 81 );
	element.setTexture( tex, &textureRect );
	setDefaultElement( UICTRL_SLIDER, 1, &element );

	// -------------------------------------
	//  CUIRadioButton

	// Box
	SetRect( &textureRect, 54, 54, 81, 81 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, false, DT_LEFT | DT_VCENTER );
	setDefaultElement( UICTRL_RADIOBUTTON, 0, &element );

	// Check
	SetRect( &textureRect, 81, 54, 108, 81 );
	element.setTexture( tex, &textureRect );
	setDefaultElement( UICTRL_RADIOBUTTON, 1, &element );


	// -------------------------------------
	//  CUIComboBox

	// main
	SetRect( &textureRect, 0, 24, 256, 49 );
	//SetRect( &textureRect, 7, 81, 247, 123 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, true );
	setDefaultElement( UICTRL_COMBOBOX, 0, &element );

	// button
	SetRect( &textureRect, 38, 394, 57, 413 );
	//SetRect( &textureRect, 272, 0, 325, 49 );
	element.setTexture( tex, &textureRect );
	//element.colorTexture.colors[ UISTATE_NORMAL ] = D3DCOLOR_ARGB(150, 255, 255, 255);
	//element.colorTexture.colors[ UISTATE_PRESSED ] = D3DCOLOR_ARGB(255, 150, 150, 150);
	//element.colorTexture.colors[ UISTATE_FOCUS ] = D3DCOLOR_ARGB(200, 255, 255, 255);
	//element.colorTexture.colors[ UISTATE_DISABLED ] = D3DCOLOR_ARGB(70, 255, 255, 255);
	setDefaultElement( UICTRL_COMBOBOX, 1, &element );

	// dropdown
	SetRect( &textureRect, 0, 106, 256, 320 );
	//SetRect( &textureRect, 7, 123, 241, 265 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, true, DT_LEFT | DT_TOP );
	element.colorTexture.colors[ UISTATE_NORMAL ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_FOCUS ] |= 0xFF000000;
	setDefaultElement( UICTRL_COMBOBOX, 2, &element );

	// selection
	SetRect( &textureRect, 0, 444, 256, 460 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, false, DT_LEFT | DT_TOP );
	//element.colorTexture.colors[ UISTATE_NORMAL ]  = 0x80000000;
	//element.colorTexture.colors[ UISTATE_FOCUS ]   = 0x80000000;
	//element.colorTexture.colors[ UISTATE_PRESSED ] = 0x80000000;
	setDefaultElement( UICTRL_COMBOBOX, 3, &element );

	// -------------------------------------
	//  CUIScrollBar

	// track
	SetRect( &textureRect, 256, 96, 272, 130 );
	element.setTexture( tex, &textureRect );
	element.colorTexture.colors[ UISTATE_NORMAL ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_FOCUS ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_DISABLED ] |= 0xFF000000;
	setDefaultElement( UICTRL_SCROLLBAR, 0, &element );

	// up arrow
	SetRect( &textureRect, 256, 81, 272, 96 );
	element.setTexture( tex, &textureRect );
	element.colorTexture.colors[ UISTATE_NORMAL ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_FOCUS ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_DISABLED ] |= 0xFF000000;
	setDefaultElement( UICTRL_SCROLLBAR, 1, &element );

	// down arrow
	SetRect( &textureRect, 256, 130, 272, 145 );
	element.setTexture( tex, &textureRect );
	element.colorTexture.colors[ UISTATE_NORMAL ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_FOCUS ] |= 0xFF000000;
	element.colorTexture.colors[ UISTATE_DISABLED ] |= 0xFF000000;
	setDefaultElement( UICTRL_SCROLLBAR, 2, &element );

	// button
	SetRect( &textureRect, 256, 146, 272, 178 );
	element.setTexture( tex, &textureRect );
	//element.colorTexture.colors[ UISTATE_NORMAL ] |= 0xFF000000;
	//element.colorTexture.colors[ UISTATE_FOCUS ] |= 0xFF000000;
	//element.colorTexture.colors[ UISTATE_DISABLED ] |= 0xFF000000;
	setDefaultElement( UICTRL_SCROLLBAR, 3, &element );


	// -------------------------------------
	//  CUIListBox

	// Main
	SetRect( &textureRect, 13, 124, 241, 265 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, true, DT_LEFT | DT_TOP );
	setDefaultElement( UICTRL_LISTBOX, 0, &element );

	// Selection
	SetRect( &textureRect, 17, 269, 241, 287 );
	element.setTexture( tex, &textureRect );
	element.setFont( 0, false, DT_LEFT | DT_TOP );
	setDefaultElement( UICTRL_LISTBOX, 1, &element );
}



// --------------------------------------------------------------------------
//  CUIResourceManager class
// --------------------------------------------------------------------------

CUIResourceManager::CUIResourceManager( int screenX, int screenY )
:	mStateBlock(NULL), mSprite(NULL),
	mScreenX( screenX ), mScreenY( screenY ),
	mInvScreenX( 1.0/screenX ), mInvScreenY( 1.0/screenY )
{
}

CUIResourceManager::~CUIResourceManager()
{
	for( int i = 0; i < mFontCache.size(); ++i )
		safeDelete( mFontCache[i] );
	mFontCache.clear();   
}

void CUIResourceManager::createResource()
{
	HRESULT hr = S_OK;
	/*
	for( int i = 0; i < mFontCache.size(); ++i ) {
		hr = createFont( i );
		assert( SUCCEEDED(hr) );
	}
	*/
	V( D3DXCreateSprite( &CD3DDevice::getInstance().getDevice(), &mSprite ) );
	assert( SUCCEEDED(hr) );
}

void CUIResourceManager::activateResource()
{
	HRESULT hr = S_OK;
	for( int i = 0; i < mFontCache.size(); ++i ) {
		hr = createFont( i );
		assert( SUCCEEDED(hr) );
		//SUIFontNode* fontNode = mFontCache[ i ];
		//if( fontNode->font )
		//	fontNode->font->OnResetDevice();
	}
	if( mSprite )
		mSprite->OnResetDevice();
	V( CD3DDevice::getInstance().getDevice().CreateStateBlock( D3DSBT_ALL, &mStateBlock ) );
	assert( SUCCEEDED(hr) );
}


void CUIResourceManager::passivateResource()
{
	for( int i = 0; i < mFontCache.size(); ++i ) {
		//SUIFontNode* fontNode = mFontCache[ i ];
		//if( fontNode->font )
		//	fontNode->font->OnLostDevice();
		safeRelease( mFontCache[ i ]->font );
	}
	if( mSprite )
		mSprite->OnLostDevice();
	safeRelease( mStateBlock );
}

void CUIResourceManager::deleteResource()
{
	// Release the resources but don't clear the cache, as these will need
	// to be recreated if the device is recreated
	//for( int i = 0; i < mFontCache.size(); ++i )
	//	safeRelease( mFontCache[ i ]->font );
	safeRelease( mSprite );
}


int CUIResourceManager::addFont( const char* facename, LONG height, LONG weight )
{
	// See if this font already exists
	for( int i=0; i < mFontCache.size(); i++ ) {
		SUIFontNode* fontNode = mFontCache[i];
		if( 0 == _strnicmp( fontNode->facename, facename, MAX_PATH-1 ) &&
			fontNode->height == height &&
			fontNode->weight == weight )
		{
			return i;
		}
	}

	// add a new font and try to create it
	SUIFontNode* newFontNode = new SUIFontNode();
	if( newFontNode == NULL )
		return -1;

	ZeroMemory( newFontNode, sizeof(SUIFontNode) );
	strncpy( newFontNode->facename, facename, MAX_PATH-1 );
	newFontNode->height = height;
	newFontNode->weight = weight;
	mFontCache.push_back( newFontNode );
	
	int fontIdx = mFontCache.size()-1;

	// If a device is available, try to create immediately
	if( CD3DDevice::getInstance().isDevice() )
		createFont( fontIdx );

	return fontIdx;
}

HRESULT CUIResourceManager::createFont( UINT fontIdx )
{
	HRESULT hr = S_OK;

	SUIFontNode* fontNode = mFontCache[ fontIdx ];

	safeRelease( fontNode->font );

	int height = xToBB( fontNode->height, CD3DDevice::getInstance().getBackBufferWidth() );
	//int height = yToBB( fontNode->height, CD3DDevice::getInstance().getBackBufferHeight() );
	V_RETURN( D3DXCreateFont( &CD3DDevice::getInstance().getDevice(), height, 0, fontNode->weight, 1, FALSE, DEFAULT_CHARSET, 
							  OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
							  fontNode->facename, &fontNode->font ) );
	return S_OK;
}



// ---------------------------------------------------------------------------
//  CUIControl class
// ---------------------------------------------------------------------------

CUIControl::CUIControl( CUIDialog *dialog )
:	mVisible(true), mMouseOver(false), mHasFocus(false), mDefault(false),
	mX(0), mY(0), mWidth(0), mHeight(0),
	mDialog(/*dialog*/NULL), mIndex(0),
	mID(0), mType(UICTRL_BUTTON), mHotkey(0), mUserData(NULL), mEnabled(true)
{
	ZeroMemory( &mBBox, sizeof( mBBox ) );
}


CUIControl::~CUIControl()
{
	stl_utils::wipe( mElements );
}


void CUIControl::setTextColor( D3DCOLOR color )
{
	SUIElement* element = mElements[ 0 ];
	if( element )
		element->colorFont.colors[UISTATE_NORMAL] = color;
}


HRESULT CUIControl::setElement( UINT index, SUIElement* element )
{
	HRESULT hr = S_OK;

	if( element == NULL )
		return E_INVALIDARG;

	// Make certain the array is this large
	for( UINT i = mElements.size(); i <= index; ++i ) {
		SUIElement* newElem = new SUIElement();
		if( newElem == NULL )
			return E_OUTOFMEMORY;
		mElements.push_back( newElem );
	}
	// Update the data
	*mElements[index] = *element;
	return S_OK;
}


void CUIControl::refresh()
{
	mMouseOver = false;
	mHasFocus = false;
	int n = mElements.size();
	for( int i = 0; i < n; ++i )
		mElements[i]->refresh();
}

void CUIControl::updateRects()
{
	mBBox.set( mX, mY, mX + mWidth, mY + mHeight );
}


// --------------------------------------------------------------------------
//  CUIStatic class
// --------------------------------------------------------------------------

CUIStatic::CUIStatic( CUIDialog *dialog )
{
	mType = UICTRL_STATIC;
	mDialog = dialog;

	ZeroMemory( &mText, sizeof(mText) );  

	int n = mElements.size();
	for( int i=0; i < n; ++i )
		safeDelete( mElements[i] );
	mElements.clear();
}


void CUIStatic::render( IDirect3DDevice9* device, float dt )
{	 
	if( !mVisible )
		return;

	eUIControlState state = UISTATE_NORMAL;
	if( !mEnabled )
		state = UISTATE_DISABLED;
		
	SUIElement* element = mElements[ 0 ];
	element->colorFont.blend( state, dt );
	mDialog->drawText( mText, element, &mBBox, !element->darkFont );
}

HRESULT CUIStatic::getTextCopy( char* strDest, UINT bufferCount )
{
	// Validate incoming parameters
	if( strDest == NULL || bufferCount == 0 )
		return E_INVALIDARG;
	// Copy the window text
	strncpy( strDest, mText, bufferCount );
	strDest[bufferCount-1] = 0;
	return S_OK;
}


HRESULT CUIStatic::setText( const char* text )
{
	if( text == NULL ) {
		mText[0] = 0;
		return S_OK;
	}
	strncpy( mText, text, MAX_PATH-1 ); 
	return S_OK;
}


// --------------------------------------------------------------------------
//  CUIImage class
// --------------------------------------------------------------------------

CUIImage::CUIImage( CUIDialog *dialog )
{
	mType = UICTRL_IMAGE;
	mDialog = dialog;

	int n = mElements.size();
	for( int i=0; i < n; ++i )
		safeDelete( mElements[i] );
	mElements.clear();
}

void CUIImage::render( IDirect3DDevice9* device, float dt )
{	 
	if( !mVisible )
		return;

	eUIControlState state = UISTATE_NORMAL;
	SUIElement* element = mElements[ 0 ];
	element->colorTexture.current = element->colorTexture.colors[state];
	mDialog->drawSprite( element, &mBBox );
}


// --------------------------------------------------------------------------
//  CUIButton class
// --------------------------------------------------------------------------

CUIButton::CUIButton( CUIDialog *dialog )
{
	mType = UICTRL_BUTTON;
	mDialog = dialog;
	mPressed = false;
	mHotkey = 0;
}

bool CUIButton::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg ) {
		case WM_KEYDOWN: {
			switch( wParam ) {
				case VK_SPACE:
					mPressed = true;
					return true;
			}
		}

		case WM_KEYUP: {
			switch( wParam ) {
				case VK_SPACE:
					if( mPressed ) {
						mPressed = false;
						mDialog->sendEvent( UIEVENT_BUTTON_CLICKED, true, this );
					}
					return true;
			}
		}
	}
	return false;
}


bool CUIButton::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg ) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			if( containsPoint( pt ) ) {
				// Pressed while inside the control
				mPressed = true;
				SetCapture( CD3DDevice::getInstance().getDeviceWindow() );
				if( !mHasFocus )
					mDialog->requestFocus( this );
				return true;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			if( mPressed ) {
				mPressed = false;
				ReleaseCapture();
				if( !mDialog->mDoKeyboardInput )
					mDialog->clearFocus();
				// Button click
				if( containsPoint( pt ) )
					mDialog->sendEvent( UIEVENT_BUTTON_CLICKED, true, this );
				return true;
			}
			break;
		}
	};
	
	return false;
}

void CUIButton::render( IDirect3DDevice9* device, float dt )
{
	int nOffsetX = 0;
	int nOffsetY = 0;

	eUIControlState state = getState();

	if( mVisible && mEnabled && mPressed ) {
		nOffsetX = 1;
		nOffsetY = 1;
	}
	
	// Background fill layer
	// TODO: remove magic numbers
	SUIElement* element = mElements[ 0 ];
	
	float blendRate = ( state == UISTATE_PRESSED ) ? 0.0f : 0.8f;

	SFRect rcWindow = mBBox;
	rcWindow.offset( nOffsetX, nOffsetY );

 	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	element->colorFont.blend( state, dt, blendRate );

	mDialog->drawSprite( element, &rcWindow );
	mDialog->drawText( mText, element, &rcWindow, !element->darkFont );

	// Main button
	/*
	element = mElements[ 1 ];

	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	element->colorFont.blend( state, dt, blendRate );

	mDialog->drawSprite( element, &rcWindow );
	mDialog->drawText( mText, element, &rcWindow, !element.darkFont );
	*/
}


eUIControlState	CUIButton::getState() const
{
	eUIControlState state = UISTATE_NORMAL;
	if( !mVisible )
		state = UISTATE_HIDDEN;
	else if( !mEnabled )
		state = UISTATE_DISABLED;
	else if( mPressed )
		state = UISTATE_PRESSED;
	else if( mMouseOver )
		state = UISTATE_MOUSEOVER;
	else if( mHasFocus )
		state = UISTATE_FOCUS;
	return state;
}



// --------------------------------------------------------------------------
//  CUICheckBox class
// --------------------------------------------------------------------------

CUICheckBox::CUICheckBox( CUIDialog *dialog )
{
	mType = UICTRL_CHECKBOX;
	mDialog = dialog;
	mChecked = false;
}
	

bool CUICheckBox::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg ) {
		case WM_KEYDOWN:
		{
			switch( wParam ) {
				case VK_SPACE:
					mPressed = true;
					return true;
			}
		}

		case WM_KEYUP:
		{
			switch( wParam ) {
				case VK_SPACE:
					if( mPressed ) {
						mPressed = false;
						setCheckedInternal( !mChecked, true );
					}
					return true;
			}
		}
	}
	return false;
}


bool CUICheckBox::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg ) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			if( containsPoint( pt ) ) {
				// Pressed while inside the control
				mPressed = true;
				SetCapture( CD3DDevice::getInstance().getDeviceWindow() );
				if( !mHasFocus && mDialog->mDoKeyboardInput )
					mDialog->requestFocus( this );
				return true;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			if( mPressed ) {
				mPressed = false;
				ReleaseCapture();
				// Button click
				if( containsPoint( pt ) )
					setCheckedInternal( !mChecked, true );
				return true;
			}
			break;
		}
	};
	
	return false;
}


void CUICheckBox::setCheckedInternal( bool chk, bool fromInput ) 
{ 
	bool changed = (chk != mChecked);
	mChecked = chk; 
	if( changed )
		mDialog->sendEvent( UIEVENT_CHECKBOX_CHANGED, fromInput, this );
}


bool CUICheckBox::containsPoint( const POINT& pt ) const
{ 
	return mBBox.containsPoint( pt ) || mRectButton.containsPoint( pt );
}


void CUICheckBox::updateRects()
{
	CUIButton::updateRects();

	mRectButton = mBBox;
	mRectButton.right = mRectButton.left + mRectButton.getHeight();

	mRectText = mBBox;
	mRectText.left += 1.25f * mRectButton.getWidth();
}


void CUICheckBox::render( IDirect3DDevice9* device, float dt )
{
	eUIControlState state = getState();

	//debug
	//mDialog->drawRect( &mBBox, D3DCOLOR_ARGB(255, 255, 255, 0) );
	//mDialog->drawRect( &mRectButton, D3DCOLOR_ARGB(255, 0, 255, 255) );

	//TODO: remove magic numbers
	SUIElement* element = mElements[ 0 ];
	
	float blendRate = ( state == UISTATE_PRESSED ) ? 0.0f : 0.8f;

	element->colorTexture.blend( state, dt, blendRate );
	element->colorFont.blend( state, dt, blendRate );

	mDialog->drawSprite( element, &mRectButton );
	mDialog->drawText( mText, element, &mRectText, !element->darkFont );

	if( !mChecked )
		state = UISTATE_HIDDEN;

	element = mElements[ 1 ];

	element->colorTexture.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectButton );
}


// --------------------------------------------------------------------------
//  CUIRollout class
// --------------------------------------------------------------------------

CUIRollout::CUIRollout( CUIDialog *dialog )
{
	mType = UICTRL_ROLLOUT;
	mDialog = dialog;
	mChecked = false;
}
	
void CUIRollout::setCheckedInternal( bool chk, bool fromInput )
{ 
	bool changed = (chk != mChecked);
	mChecked = chk; 
	updateRects();
	if( changed )
		mDialog->sendEvent( UIEVENT_CHECKBOX_CHANGED, fromInput, this ); 
}

bool CUIRollout::containsPoint( const POINT& pt ) const
{ 
	return mBBox.containsPoint( pt ) || mRectText.containsPoint( pt );
}


void CUIRollout::updateRects()
{
	CUIButton::updateRects();

	mRectButton = mBBox;

	float shiftup = mBBox.getHeight() / 8;
	mRectText = mBBox;
	mRectText.top = mBBox.bottom - shiftup;
	mRectText.bottom = mBBox.bottom + (mChecked ? mRolloutHeight : 0);
}

void CUIRollout::render( IDirect3DDevice9* device, float dt )
{
	eUIControlState state = getState();

	// TODO: remove magic numbers
	float blendRate = ( state == UISTATE_PRESSED ) ? 0.0f : 0.4f;

	SUIElement* element = mElements[ 1 ];
	element->colorTexture.blend( !isExpanded() ? UISTATE_HIDDEN : state, dt, blendRate );
	if( isExpanded() )
		mDialog->drawSprite( element, &mRectText );

	element = mElements[ 0 ];
	element->colorTexture.blend( state, dt, blendRate );
	element->colorFont.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectButton );
	mDialog->drawText( mText, element, &mRectButton, !element->darkFont );
}


// --------------------------------------------------------------------------
//  CUIRadioButton class
// --------------------------------------------------------------------------

CUIRadioButton::CUIRadioButton( CUIDialog *dialog )
{
	mType = UICTRL_RADIOBUTTON;
	mDialog = dialog;
}


bool CUIRadioButton::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg ) {
		case WM_KEYDOWN:
		{
			switch( wParam ) {
				case VK_SPACE:
					mPressed = true;
					return true;
			}
		}

		case WM_KEYUP:
		{
			switch( wParam ) {
				case VK_SPACE:
					if( mPressed ) {
						mPressed = false;
						mDialog->clearRadioButtonGroup( mButtonGroup );
						mChecked = !mChecked;
						mDialog->sendEvent( UIEVENT_RADIOBUTTON_CHANGED, true, this );
					}
					return true;
			}
		}
	}
	return false;
}


bool CUIRadioButton::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg ) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			if( containsPoint( pt ) ) {
				// Pressed while inside the control
				mPressed = true;
				SetCapture( CD3DDevice::getInstance().getDeviceWindow() );
				if( !mHasFocus && mDialog->mDoKeyboardInput )
					mDialog->requestFocus( this );
				return true;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			if( mPressed ) {
				mPressed = false;
				ReleaseCapture();
				// Button click
				if( containsPoint( pt ) ) {
					mDialog->clearRadioButtonGroup( mButtonGroup );
					mChecked = !mChecked;
					mDialog->sendEvent( UIEVENT_RADIOBUTTON_CHANGED, true, this );
				}
				return true;
			}
			break;
		}
	};
	
	return false;
}

void CUIRadioButton::setCheckedInternal( bool chk, bool clearGroup, bool fromInput )
{
	if( chk && clearGroup )
		mDialog->clearRadioButtonGroup( mButtonGroup );
	mChecked = chk;
	mDialog->sendEvent( UIEVENT_RADIOBUTTON_CHANGED, fromInput, this );
}



// --------------------------------------------------------------------------
//  CUIComboBox class
// --------------------------------------------------------------------------

CUIComboBox::CUIComboBox( CUIDialog *dialog )
:	mScrollBar( dialog )
{
	mType = UICTRL_COMBOBOX;
	mDialog = dialog;
	mDropHeight = 75;
	mSBWidth = 12;
	mOpened = false;
	mSelected = -1;
	mFocused = -1;
}


CUIComboBox::~CUIComboBox()
{
	removeAllItems();
}


void CUIComboBox::setTextColor( D3DCOLOR color )
{
	SUIElement* element = mElements[ 0 ];
	if( element )
		element->colorFont.colors[UISTATE_NORMAL] = color;
	element = mElements[ 2 ];
	if( element )
		element->colorFont.colors[UISTATE_NORMAL] = color;
}


void CUIComboBox::updateRects()
{
	CUIButton::updateRects();

	mRectButton = mBBox;
	mRectButton.left = mRectButton.right - mRectButton.getHeight();

	mRectText = mBBox;
	//mRectText.right = mRectButton.left;

	mRectDropdown = mRectText;
	mRectDropdown.offset( 0, 0.90f * mRectText.getHeight() );
	mRectDropdown.bottom += mDropHeight;
	mRectDropdown.right -= mSBWidth;

	float hgt = mBBox.getHeight();
	mRectDropdownText = mRectDropdown;
	mRectDropdownText.left += hgt/4;
	mRectDropdownText.right -= hgt/4;
	mRectDropdownText.top += hgt/4;
	mRectDropdownText.bottom -= hgt/4;

	// Update the scrollbar's rects
	mScrollBar.setLocation( mRectDropdown.right, mRectDropdown.top+2 );
	mScrollBar.setSize( mSBWidth, mRectDropdown.getHeight() - 2 );
	SUIFontNode* fontNode = CUIResourceManager::getInstance().getFontNode( mElements[ 2 ]->fontIdx );
	if( fontNode && fontNode->height ) {
		mScrollBar.setPageSize( mRectDropdownText.getHeight() / fontNode->height );
		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		mScrollBar.showItem( mSelected );
	}
}


void CUIComboBox::onFocusOut()
{
	CUIButton::onFocusOut();
	mOpened = false;
}
	

bool CUIComboBox::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	const DWORD REPEAT_MASK = (0x40000000);

	if( !mEnabled || !mVisible )
		return false;

	// Let the scroll bar have a chance to handle it first
	if( mScrollBar.handleKeyb( msg, wParam, lParam ) )
		return true;

	switch( msg ) {
		case WM_KEYDOWN:
		{
			switch( wParam ) {
				case VK_RETURN:
					if( mOpened ) {
						if( mSelected != mFocused ) {
							mSelected = mFocused;
							mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
						}
						mOpened = false;
						if( !mDialog->mDoKeyboardInput )
							mDialog->clearFocus();
						return true;
					}
					break;
				case VK_F4:
					// Filter out auto-repeats
					if( lParam & REPEAT_MASK )
						return true;
					mOpened = !mOpened;
					if( !mOpened ) {
						mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
						if( !mDialog->mDoKeyboardInput )
							mDialog->clearFocus();
					}
					return true;
				case VK_LEFT:
				case VK_UP:
					if( mFocused > 0 ) {
						mFocused--;
						mSelected = mFocused;
						if( !mOpened )
							mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
					}
					return true;

				case VK_RIGHT:
				case VK_DOWN:
					if( mFocused+1 < (int)getItemCount() ) {
						++mFocused;
						mSelected = mFocused;
						if( !mOpened )
							mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
					}
					return true;
			}
			break;
		}
	}

	return false;
}


bool CUIComboBox::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	// Let the scroll bar handle it first.
	if( mOpened && mScrollBar.handleMouse( msg, pt, wParam, lParam ) )
		return true;

	switch( msg ) {
		case WM_MOUSEMOVE:
		{
			if( mOpened && mRectDropdown.containsPoint( pt ) ) {
				// Determine which item has been selected
				for( int i=0; i < mItems.size(); i++ ) {
					SUIComboItem* item = mItems[ i ];
					if( item -> vis && item->activeRect.containsPoint( pt ) ) {
						mFocused = i;
					}
				}
				return true;
			}
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			if( containsPoint( pt ) ) {
				// Pressed while inside the control
				mPressed = true;
				SetCapture( CD3DDevice::getInstance().getDeviceWindow() );

				if( !mHasFocus )
					mDialog->requestFocus( this );

				// Toggle dropdown
				if( mHasFocus ) {
					mOpened = !mOpened;
					if( !mOpened ) {
						if( !mDialog->mDoKeyboardInput )
							mDialog->clearFocus();
					}
				}
				return true;
			}

			// Perhaps this click is within the dropdown
			if( mOpened && mRectDropdown.containsPoint( pt ) ) {
				// Determine which item has been selected
				for( int i=mScrollBar.getTrackPos(); i < mItems.size(); i++ ) {
					SUIComboItem* item = mItems[ i ];
					if( item -> vis && item->activeRect.containsPoint( pt ) ) {
						mFocused = mSelected = i;
						mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
						mOpened = false;
						if( !mDialog->mDoKeyboardInput )
							mDialog->clearFocus();
						break;
					}
				}
				return true;
			}

			// Mouse click not on main control or in dropdown, fire an event if needed
			if( mOpened ) {
				mFocused = mSelected;
				mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
				mOpened = false;
			}

			// Make sure the control is no longer in a pressed state
			mPressed = false;

			// Release focus if appropriate
			if( !mDialog->mDoKeyboardInput ) {
				mDialog->clearFocus();
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			if( mPressed && containsPoint( pt ) ) {
				// Button click
				mPressed = false;
				ReleaseCapture();
				return true;
			}
			break;
		}

		case WM_MOUSEWHEEL:
		{
			int zDelta = (short) HIWORD(wParam) / WHEEL_DELTA;
			if( mOpened ) {
				UINT lines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &lines, 0 );
				mScrollBar.scroll( -zDelta * lines );
			} else {
				if( zDelta > 0 ) {
					if( mFocused > 0 ) {
						mFocused--;
						mSelected = mFocused;	  
						if( !mOpened )
							mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
					}		   
				} else {
					if( mFocused+1 < (int)getItemCount() ) {
						mFocused++;
						mSelected = mFocused;	
						if( !mOpened )
							mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
					}
				}
			}
			return true;
		}
	};
	return false;
}


void CUIComboBox::onHotkey()
{
	if( mOpened )
		return;
	if( mSelected == -1 )
		return;
	mSelected++;
	if( mSelected >= (int) mItems.size() )
		mSelected = 0;
	mFocused = mSelected;
	mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, true, this );
}


void CUIComboBox::render( IDirect3DDevice9* device, float dt )
{
	eUIControlState state = UISTATE_NORMAL;
	
	if( !mOpened )
		state = UISTATE_HIDDEN;

	//
	// dropdown box

	SUIElement* element = mElements[ 2 ];

	// If we have not initialized the scroll bar page size, do that now.
	static bool sbInit;
	if( !sbInit ) {
		// Update the page size of the scroll bar
		if( CUIResourceManager::getInstance().getFontNode( element->fontIdx )->height )
			mScrollBar.setPageSize( mRectDropdownText.getHeight() / CUIResourceManager::getInstance().getFontNode( element->fontIdx )->height );
		else
			mScrollBar.setPageSize( mRectDropdownText.getHeight() );
		sbInit = true;
	}

	// scroll bar
	if( mOpened )
		mScrollBar.render( device, dt );

	// blend current color
	element->colorTexture.blend( state, dt );
	element->colorFont.blend( state, dt );

	mDialog->drawSprite( element, &mRectDropdown );

	//
	// selection outline

	SUIElement* selElement = mElements[ 3 ];
	//selElement->colorTexture.current = element->colorTexture.current;
	selElement->colorTexture.current = selElement->colorTexture.colors[ UISTATE_NORMAL ];
	selElement->colorFont.current = selElement->colorFont.colors[ UISTATE_NORMAL ];

	SUIFontNode* font = mDialog->getFont( element->fontIdx );
	float curY = mRectDropdownText.top;
	float nRemainingHeight = mRectDropdownText.getHeight();

	for( int i = mScrollBar.getTrackPos(); i < mItems.size(); ++i ) {
		SUIComboItem* item = mItems[ i ];

		// Make sure there's room left in the dropdown
		nRemainingHeight -= font->height;
		if( nRemainingHeight < 0 ) {
			item->vis = false;
			continue;
		}

		item->activeRect.set( mRectDropdownText.left, curY, mRectDropdownText.right, curY + font->height );
		curY += font->height;
		
		//debug
		//int blue = 50 * i;
		//mDialog->drawRect( &item->activeRect, 0xFFFF0000 | blue );

		item->vis = true;

		if( mOpened ) {
			if( (int)i == mFocused ) {
				SFRect rc;
				rc.set( mRectDropdown.left, item->activeRect.top-2, mRectDropdown.right, item->activeRect.bottom+2 );
				mDialog->drawSprite( selElement, &rc );
				mDialog->drawText( item->text, selElement, &item->activeRect, !selElement->darkFont );
			} else {
				mDialog->drawText( item->text, element, &item->activeRect, !element->darkFont );
			}
		}
	}

	//
	// main & button

	int nOffsetX = 0;
	int nOffsetY = 0;

	state = UISTATE_NORMAL;
	
	if( !mVisible )
		state = UISTATE_HIDDEN;
	else if( !mEnabled )
		state = UISTATE_DISABLED;
	else if( mPressed ) {
		state = UISTATE_PRESSED;
		nOffsetX = 1;
		nOffsetY = 1;
	} else if( mMouseOver ) {
		state = UISTATE_MOUSEOVER;
		//nOffsetX = -1;
		//nOffsetY = -1;
	} else if( mHasFocus )
		state = UISTATE_FOCUS;

	float blendRate = ( state == UISTATE_PRESSED ) ? 0.0f : 0.8f;

	// main box
	if( mOpened )
		state = UISTATE_PRESSED;

	element = mElements[ 0 ];
	element->colorTexture.blend( state, dt, blendRate );
	element->colorFont.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectText);
	
	if( mSelected >= 0 && mSelected < (int) mItems.size() ) {
		SUIComboItem* item = mItems[ mSelected ];
		if( item != NULL ) {
			mDialog->drawText( item->text, element, &mRectText, !element->darkFont );
		}
	}
	
	// button
	element = mElements[ 1 ];
	element->colorTexture.blend( state, dt, blendRate );
	SFRect rcWindow = mRectButton;
	rcWindow.offset( nOffsetX, nOffsetY );
	mDialog->drawSprite( element, &rcWindow );
}


HRESULT CUIComboBox::addItem( const char* text, const void* data, bool updateSelectionIf1st )
{
	// Validate parameters
	if( text== NULL )
		return E_INVALIDARG;
	
	// Create a new item and set the data
	SUIComboItem* item = new SUIComboItem;
	if( item == NULL ) {
		ASSERT_FAIL_MSG( "new" );
		return E_OUTOFMEMORY;
	}
	
	ZeroMemory( item, sizeof(SUIComboItem) );
	strncpy( item->text, text, 255 );
	item->data = data;

	mItems.push_back( item );

	// Update the scroll bar with new range
	mScrollBar.setTrackRange( 0, mItems.size() );

	// If this is the only item in the list, it's selected
	if( updateSelectionIf1st && getItemCount() == 1 ) {
		mSelected = 0;
		mFocused = 0;
		mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, false, this );
	}

	return S_OK;
}


void CUIComboBox::removeItem( UINT index )
{
	SUIComboItem* item = mItems[ index ];
	safeDelete( item );
	mItems.erase( mItems.begin() + index );
	mScrollBar.setTrackRange( 0, mItems.size() );
	if( mSelected >= mItems.size() )
		mSelected = mItems.size() - 1;
}


void CUIComboBox::removeAllItems()
{
	for( int i=0; i < mItems.size(); i++ )
		safeDelete( mItems[i] );
	mItems.clear();
	mScrollBar.setTrackRange( 0, 1 );
	mFocused = mSelected = -1;
}

bool CUIComboBox::containsItem( const char* text, UINT start ) const
{
	return ( -1 != findItem( text, start ) );
}


int CUIComboBox::findItem( const char* text, UINT start ) const
{
	if( text == NULL )
		return -1;

	for( int i = start; i < mItems.size(); i++ ) {
		SUIComboItem* item = mItems[i];
		if( 0 == strcmp( item->text, text ) ) {
			return i;
		}
	}

	return -1;
}


const void* CUIComboBox::getSelectedData()
{
	if( mSelected < 0 )
		return NULL;
	SUIComboItem* item = mItems[ mSelected ];
	return item->data;
}


SUIComboItem* CUIComboBox::getSelectedItem()
{
	if( mSelected < 0 )
		return NULL;

	return mItems[ mSelected ];
}


const void* CUIComboBox::getItemData( const char* text )
{
	int index = findItem( text );
	if( index == -1 )
		return NULL;

	SUIComboItem* item = mItems[index];
	if( item == NULL ) {
		ASSERT_FAIL_MSG( "" );
		return NULL;
	}
	return item->data;
}


HRESULT CUIComboBox::setSelectedByIndex( UINT index )
{
	if( index >= getItemCount() )
		return E_INVALIDARG;

	mFocused = mSelected = index;
	mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, false, this );

	return S_OK;
}


HRESULT CUIComboBox::setSelectedByText( const char* text )
{
	if( text == NULL )
		return E_INVALIDARG;

	int index = findItem( text );
	if( index == -1 )
		return E_FAIL;

	mFocused = mSelected = index;
	mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, false, this );

	return S_OK;
}


HRESULT CUIComboBox::setSelectedByData( const void* data )
{
	for( int i=0; i < mItems.size(); i++ ) {
		SUIComboItem* item = mItems[i];
		if( item->data == data ) {
			mFocused = mSelected = i;
			mDialog->sendEvent( UIEVENT_COMBOBOX_SELECTION_CHANGED, false, this );
			return S_OK;
		}
	}
	return E_FAIL;
}



// --------------------------------------------------------------------------
//  CUISlider control
// --------------------------------------------------------------------------

CUISlider::CUISlider( CUIDialog *dialog )
{
	mType = UICTRL_SLIDER;
	mDialog = dialog;
	mMin = 0;
	mMax = 100;
	mValue = 50;
	mPressed = false;
}


bool CUISlider::containsPoint( const POINT& pt ) const
{ 
	return mBBox.containsPoint( pt ) || mRectButton.containsPoint( pt );
}


void CUISlider::updateRects()
{
	CUIControl::updateRects();

	mRectButton = mBBox;
	mRectButton.right = mRectButton.left + mRectButton.getHeight();
	mRectButton.offset( -mRectButton.getWidth()/2, 0 );

	mButtonX = ( (mValue - mMin) * mBBox.getWidth() / (mMax - mMin) );
	mRectButton.offset( mButtonX, 0 );
}

int CUISlider::getValueFromPos( int x )
{ 
	float valPerCoord = (float)(mMax - mMin) / mBBox.getWidth();
	return (int) (0.5f + mMin + valPerCoord * (x - mBBox.left));
}

bool CUISlider::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg )
	{
		case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_HOME:
					setValueInternal( mMin, true );
					return true;

				case VK_END:
					setValueInternal( mMax, true );
					return true;

				case VK_PRIOR:
				case VK_LEFT:
				case VK_UP:
					setValueInternal( mValue - 1, true );
					return true;

				case VK_NEXT:
				case VK_RIGHT:
				case VK_DOWN:
					setValueInternal( mValue + 1, true );
					return true;
			}
			break;
		}
	}
	

	return false;
}


//--------------------------------------------------------------------------------------
bool CUISlider::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	switch( msg )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			if( mRectButton.containsPoint( pt ) ) {
				// Pressed while inside the control
				mPressed = true;
				SetCapture( CD3DDevice::getInstance().getDeviceWindow() );

				mDragStartX = pt.x;
				//m_nDragY = pt.y;
				mDragOffset = mButtonX - mDragStartX;

				//m_nDragValue = mValue;

				if( !mHasFocus )
					mDialog->requestFocus( this );

				return true;
			}

			if( mBBox.containsPoint( pt ) ) {
			   if( pt.x > mButtonX + mX ) {
				   setValueInternal( mValue + 1, true );
				   return true;
			   }

			   if( pt.x < mButtonX + mX ) {
				   setValueInternal( mValue - 1, true );
				   return true;
			   }
			}

			break;
		}

		case WM_LBUTTONUP:
		{
			if( mPressed )
			{
				mPressed = false;
				ReleaseCapture();
				mDialog->clearFocus();
				mDialog->sendEvent( UIEVENT_SLIDER_VALUE_CHANGED, true, this );

				return true;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			if( mPressed )
			{
				setValueInternal( getValueFromPos( mX + pt.x + mDragOffset ), true );
				return true;
			}

			break;
		}
	};
	
	return false;
}

void CUISlider::setRange( int vmin, int vmax ) 
{
	mMin = vmin; 
	mMax = vmax; 

	setValueInternal( mValue, false );
}

void CUISlider::setValueInternal( int value, bool fromInput )
{
	// Clamp to range
	value = max( mMin, value );
	value = min( mMax, value );
	
	if( value == mValue )
		return;

	mValue = value;
	updateRects();
	mDialog->sendEvent( UIEVENT_SLIDER_VALUE_CHANGED, fromInput, this );
}


void CUISlider::render( IDirect3DDevice9* device, float dt )
{
	int nOffsetX = 0;
	int nOffsetY = 0;

	eUIControlState state = UISTATE_NORMAL;

	if( !mVisible )
	{
		state = UISTATE_HIDDEN;
	}
	else if( !mEnabled )
	{
		state = UISTATE_DISABLED;
	}
	else if( mPressed )
	{
		state = UISTATE_PRESSED;

		nOffsetX = 1;
		nOffsetY = 2;
	}
	else if( mMouseOver )
	{
		state = UISTATE_MOUSEOVER;
		
		nOffsetX = -1;
		nOffsetY = -2;
	}
	else if( mHasFocus )
	{
		state = UISTATE_FOCUS;
	}

	float blendRate = ( state == UISTATE_PRESSED ) ? 0.0f : 0.8f;

	SUIElement* element = mElements[ 0 ];
	
	// blend current color
	element->colorTexture.blend( state, dt, blendRate ); 
	mDialog->drawSprite( element, &mBBox );

	//TODO: remove magic numbers
	element = mElements[ 1 ];
	   
	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectButton );
}


//--------------------------------------------------------------------------------------
// CUIScrollBar class
//--------------------------------------------------------------------------------------

CUIScrollBar::CUIScrollBar( CUIDialog *dialog )
{
	mType = UICTRL_SCROLLBAR;
	mDialog = dialog;

	mShowThumb = true;

	mRectUp.set( 0, 0, 0, 0 );
	mRectDown.set( 0, 0, 0, 0 );
	mRectTrack.set( 0, 0, 0, 0 );
	mRectThumb.set( 0, 0, 0, 0 );
	mPosition = 0;
	mPageSize = 1;
	mStart = 0;
	mEnd = 1;
	mArrows = CLEAR;
	mLastArrowTime = 0.0;
}


CUIScrollBar::~CUIScrollBar()
{
}


void CUIScrollBar::updateRects()
{
	CUIControl::updateRects();

	// Make the buttons square
	mRectUp.set( mBBox.left, mBBox.top, mBBox.right, mBBox.top + mBBox.getWidth() );
	mRectDown.set( mBBox.left, mBBox.bottom - mBBox.getWidth(), mBBox.right, mBBox.bottom );
	mRectTrack.set( mRectUp.left, mRectUp.bottom, mRectDown.right, mRectDown.top );
	mRectThumb.left = mRectUp.left;
	mRectThumb.right = mRectUp.right;

	updateThumbRect();
}


void CUIScrollBar::updateThumbRect()
{
	if( mEnd - mStart > mPageSize ) {
		int nThumbHeight = max( mRectTrack.getHeight() * mPageSize / ( mEnd - mStart ), SCROLLBAR_MINTHUMBSIZE );
		nThumbHeight = min( nThumbHeight, SCROLLBAR_MAXTHUMBSIZE );
		int nMaxPosition = mEnd - mStart - mPageSize;
		mRectThumb.top = mRectTrack.top + ( mPosition - mStart ) * ( mRectTrack.getHeight() - nThumbHeight ) / nMaxPosition;
		mRectThumb.bottom = mRectThumb.top + nThumbHeight;
		mShowThumb = true;
	} else {
		// No content to scroll
		mRectThumb.top = mRectTrack.top;
		mRectThumb.bottom = mRectThumb.top;
		mShowThumb = false;
	}
}


// scroll() scrolls by delta items.  A positive value scrolls down, while
// a negative value scrolls up.
void CUIScrollBar::scroll( int delta )
{
	// Perform scroll
	mPosition += delta;

	// capPosition position
	capPosition();

	// Update thumb position
	updateThumbRect();
}


void CUIScrollBar::showItem( int index )
{
	// capPosition the index

	if( index < 0 )
		index = 0;

	if( index >= mEnd )
		index = mEnd - 1;

	// Adjust position

	if( mPosition > index )
		mPosition = index;
	else
	if( mPosition + mPageSize <= index )
		mPosition = index - mPageSize + 1;

	updateThumbRect();
}


bool CUIScrollBar::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	return false;
}


bool CUIScrollBar::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	static int ThumbOffsetY;
	static bool dragging;

	mLastMouse = pt;
	switch( msg )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			SetCapture( CD3DDevice::getInstance().getDeviceWindow() );

			// Check for click on up button
			if( mRectUp.containsPoint( pt ) ) {
				if( mPosition > mStart )
					--mPosition;
				updateThumbRect();
				mArrows = CLICKED_UP;
				mLastArrowTime = CSystemTimer::getInstance().getTimeS();
				return true;
			}

			// Check for click on down button
			if( mRectDown.containsPoint( pt ) ) {
				if( mPosition + mPageSize < mEnd )
					++mPosition;
				updateThumbRect();
				mArrows = CLICKED_DOWN;
				mLastArrowTime = CSystemTimer::getInstance().getTimeS();
				return true;
			}

			// Check for click on thumb
			if( mRectThumb.containsPoint( pt ) ) {
				dragging = true;
				ThumbOffsetY = pt.y - mRectThumb.top;
				return true;
			}

			// Check for click on track
			if( mRectThumb.left <= pt.x && mRectThumb.right > pt.x ) {
				if( mRectThumb.top > pt.y && mRectTrack.top <= pt.y ) {
					scroll( -( mPageSize - 1 ) );
					return true;
				} else if( mRectThumb.bottom <= pt.y && mRectTrack.bottom > pt.y ) {
					scroll( mPageSize - 1 );
					return true;
				}
			}

			break;
		}

		case WM_LBUTTONUP: {
			dragging = false;
			ReleaseCapture();
			updateThumbRect();
			mArrows = CLEAR;
			break;
		}

		case WM_MOUSEMOVE: {
			if( dragging ) {
				mRectThumb.bottom += pt.y - ThumbOffsetY - mRectThumb.top;
				mRectThumb.top = pt.y - ThumbOffsetY;
				if( mRectThumb.top < mRectTrack.top )
					mRectThumb.offset( 0, mRectTrack.top - mRectThumb.top );
				else if( mRectThumb.bottom > mRectTrack.bottom )
					mRectThumb.offset( 0, mRectTrack.bottom - mRectThumb.bottom );

				// Compute first item index based on thumb position
				int nMaxFirstItem = mEnd - mStart - mPageSize;  // Largest possible index for first item
				int nMaxThumb = mRectTrack.getHeight() - mRectThumb.getHeight();	// Largest possible thumb position from the top

				mPosition = mStart +
							  ( mRectThumb.top - mRectTrack.top +
								nMaxThumb / ( nMaxFirstItem * 2 ) ) * // Shift by half a row to avoid last row covered by only one pixel
							  nMaxFirstItem  / nMaxThumb;

				return true;
			}
			break;
		}
	}

	return false;
}


void CUIScrollBar::render( IDirect3DDevice9* device, float dt )
{
	// Check if the arrow button has been held for a while.
	// If so, update the thumb position to simulate repeated
	// scroll.
	if( mArrows != CLEAR )
	{
		double dCurrTime = CSystemTimer::getInstance().getTimeS();
		if( mRectUp.containsPoint( mLastMouse ) ) {
			switch( mArrows )
			{
				case CLICKED_UP:
					if( SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - mLastArrowTime )
					{
						scroll( -1 );
						mArrows = HELD_UP;
						mLastArrowTime = dCurrTime;
					}
					break;
				case HELD_UP:
					if( SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - mLastArrowTime )
					{
						scroll( -1 );
						mLastArrowTime = dCurrTime;
					}
					break;
			}
		} else if( mRectDown.containsPoint( mLastMouse ) ) {
			switch( mArrows ) {
				case CLICKED_DOWN:
					if( SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - mLastArrowTime )
					{
						scroll( 1 );
						mArrows = HELD_DOWN;
						mLastArrowTime = dCurrTime;
					}
					break;
				case HELD_DOWN:
					if( SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - mLastArrowTime )
					{
						scroll( 1 );
						mLastArrowTime = dCurrTime;
					}
					break;
			}
		}
	}

	eUIControlState state = UISTATE_NORMAL;

	if( !mVisible )
		state = UISTATE_HIDDEN;
	else if( !mEnabled || mShowThumb == false )
		state = UISTATE_DISABLED;
	else if( mMouseOver )
		state = UISTATE_MOUSEOVER;
	else if( mHasFocus )
		state = UISTATE_FOCUS;


	float blendRate = ( state == UISTATE_PRESSED ) ? 0.0f : 0.8f;

	// Background track layer
	SUIElement* element = mElements[ 0 ];
	
	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectTrack );

	// Up Arrow
	element = mElements[ 1 ];
	
	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectUp );

	// Down Arrow
	element = mElements[ 2 ];
	
	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectDown );

	// Thumb button
	element = mElements[ 3 ];
	
	// blend current color
	element->colorTexture.blend( state, dt, blendRate );
	mDialog->drawSprite( element, &mRectThumb );
 
}


//--------------------------------------------------------------------------------------
void CUIScrollBar::setTrackRange( int start, int end )
{
	mStart = start; mEnd = end;
	capPosition();
	updateThumbRect();
}


//--------------------------------------------------------------------------------------
void CUIScrollBar::capPosition()	// Clips position at boundaries. Ensures it stays within legal range.
{
	if( mPosition < mStart ||
		mEnd - mStart <= mPageSize )
	{
		mPosition = mStart;
	}
	else
	if( mPosition + mPageSize > mEnd )
		mPosition = mEnd - mPageSize;
}

//--------------------------------------------------------------------------------------
// CUIListBox class
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
CUIListBox::CUIListBox( CUIDialog *dialog ) :
	mScrollBar( dialog )
{
	mType = UICTRL_LISTBOX;
	mDialog = dialog;

	mStyle = 0;
	mSBWidth = 16;
	mSelected = -1;
	mSelStart = 0;
	mDragging = false;
	mBorder = 6;
	mMargin = 5;
	mTextHeight = 0;
}


CUIListBox::~CUIListBox()
{
	removeAllItems();
}

void CUIListBox::updateRects()
{
	CUIControl::updateRects();

	mRectSelection = mBBox;
	mRectSelection.right -= mSBWidth;
	mRectSelection.inflate( -mBorder, -mBorder );
	mRectText = mRectSelection;
	mRectText.inflate( -mMargin, 0 );

	// Update the scrollbar's rects
	mScrollBar.setLocation( mBBox.right - mSBWidth, mBBox.top );
	mScrollBar.setSize( mSBWidth, mHeight );
	SUIFontNode* fontNode = CUIResourceManager::getInstance().getFontNode( mElements[ 0 ]->fontIdx );
	if( fontNode && fontNode->height ) {
		mScrollBar.setPageSize( mRectText.getHeight() / fontNode->height );

		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		mScrollBar.showItem( mSelected );
	}
}


// --------------------------------------------------------------------------

HRESULT CUIListBox::addItem( const char *text, const void *data )
{
	SUIListItem *pNewItem = new SUIListItem;
	if( !pNewItem )
		return E_OUTOFMEMORY;

	strncpy( pNewItem->text, text, 256 );
	pNewItem->text[255] = L'\0';
	pNewItem->data = data;
	pNewItem->activeRect.set( 0, 0, 0, 0 );
	pNewItem->selected = false;

	mItems.push_back( pNewItem );
	mScrollBar.setTrackRange( 0, mItems.size() );

	return S_OK;
}

// --------------------------------------------------------------------------

HRESULT CUIListBox::insertItem( int index, const char *text, const void *data )
{
	SUIListItem *pNewItem = new SUIListItem;
	if( !pNewItem )
		return E_OUTOFMEMORY;

	strncpy( pNewItem->text, text, 256 );
	pNewItem->text[255] = L'\0';
	pNewItem->data = data;
	pNewItem->activeRect.set( 0, 0, 0, 0 );
	pNewItem->selected = false;

	mItems.insert( mItems.begin() + index, pNewItem );
	mScrollBar.setTrackRange( 0, mItems.size() );

	return S_OK;
}


//--------------------------------------------------------------------------------------
void CUIListBox::removeItem( int index )
{
	if( index < 0 || index >= (int)mItems.size() )
		return;

	SUIListItem *item = mItems[ index ];

	delete item;
	mItems.erase( mItems.begin() + index );
	mScrollBar.setTrackRange( 0, mItems.size() );
	if( mSelected >= (int)mItems.size() )
		mSelected = mItems.size() - 1;

	mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
}


void CUIListBox::removeItemByText( const char *text )
{
	// TBD - implementation?
}


void CUIListBox::removeItemByData( const void *data )
{
	// TBD - implementation?
}


void CUIListBox::removeAllItems()
{
	for( int i = 0; i < mItems.size(); ++i )
	{
		SUIListItem *item = mItems[ i ];
		delete item;
	}

	mItems.clear();
	mScrollBar.setTrackRange( 0, 1 );
}


SUIListItem *CUIListBox::getItem( int index )
{
	if( index < 0 || index >= (int)mItems.size() )
		return NULL;

	return mItems[index];
}


//--------------------------------------------------------------------------------------
// For single-selection listbox, returns the index of the selected item.
// For multi-selection, returns the first selected item after the prevSelected position.
// To search for the first selected item, the app passes -1 for prevSelected.	For
// subsequent searches, the app passes the returned index back to getSelectedIndex as.
// prevSelected.
// Returns -1 on error or if no item is selected.
int CUIListBox::getSelectedIndex( int prevSelected ) const
{
	if( prevSelected < -1 )
		return -1;

	if( mStyle & MULTISELECTION )
	{
		// Multiple selection enabled. Search for the next item with the selected flag.
		for( int i = prevSelected + 1; i < (int)mItems.size(); ++i )
		{
			SUIListItem *item = mItems[ i ];

			if( item->selected )
				return i;
		}

		return -1;
	}
	else
	{
		// Single selection
		return mSelected;
	}
}


//--------------------------------------------------------------------------------------
void CUIListBox::selectItem( int newIndex )
{
	// If no item exists, do nothing.
	if( mItems.size() == 0 )
		return;

	int nOldSelected = mSelected;

	// Adjust mSelected
	mSelected = newIndex;

	// Perform capping
	if( mSelected < 0 )
		mSelected = 0;
	if( mSelected >= (int)mItems.size() )
		mSelected = mItems.size() - 1;

	if( nOldSelected != mSelected )
	{
		if( mStyle & MULTISELECTION )
		{
			mItems[mSelected]->selected = true;
		}

		// Update selection start
		mSelStart = mSelected;

		// Adjust scroll bar
		mScrollBar.showItem( mSelected );
	}

	mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
}


//--------------------------------------------------------------------------------------
bool CUIListBox::handleKeyb( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	// Let the scroll bar have a chance to handle it first
	if( mScrollBar.handleKeyb( msg, wParam, lParam ) )
		return true;

	switch( msg )
	{
		case WM_KEYDOWN:
			switch( wParam )
			{
				case VK_UP:
				case VK_DOWN:
				case VK_NEXT:
				case VK_PRIOR:
				case VK_HOME:
				case VK_END:

					// If no item exists, do nothing.
					if( mItems.size() == 0 )
						return true;

					int nOldSelected = mSelected;

					// Adjust mSelected
					switch( wParam )
					{
						case VK_UP: --mSelected; break;
						case VK_DOWN: ++mSelected; break;
						case VK_NEXT: mSelected += mScrollBar.getPageSize() - 1; break;
						case VK_PRIOR: mSelected -= mScrollBar.getPageSize() - 1; break;
						case VK_HOME: mSelected = 0; break;
						case VK_END: mSelected = mItems.size() - 1; break;
					}

					// Perform capping
					if( mSelected < 0 )
						mSelected = 0;
					if( mSelected >= (int)mItems.size() )
						mSelected = mItems.size() - 1;

					if( nOldSelected != mSelected )
					{
						if( mStyle & MULTISELECTION )
						{
							// Multiple selection

							// clear all selection
							for( int i = 0; i < (int)mItems.size(); ++i )
							{
								SUIListItem *item = mItems[i];
								item->selected = false;
							}

							if( GetKeyState( VK_SHIFT ) < 0 )
							{
								// Select all items from mSelStart to
								// mSelected
								int end = max( mSelStart, mSelected );

								for( int n = min( mSelStart, mSelected ); n <= end; ++n )
									mItems[n]->selected = true;
							}
							else
							{
								mItems[mSelected]->selected = true;

								// Update selection start
								mSelStart = mSelected;
							}
						} else
							mSelStart = mSelected;

						// Adjust scroll bar

						mScrollBar.showItem( mSelected );

						// Send notification

						mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
					}
					return true;
			}
			break;
	}

	return false;
}


//--------------------------------------------------------------------------------------
bool CUIListBox::handleMouse( UINT msg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !mEnabled || !mVisible )
		return false;

	// First acquire focus
	if( WM_LBUTTONDOWN == msg )
		if( !mHasFocus )
			mDialog->requestFocus( this );

	// Let the scroll bar handle it first.
	if( mScrollBar.handleMouse( msg, pt, wParam, lParam ) )
		return true;

	switch( msg )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			// Check for clicks in the text area
			if( mItems.size() > 0 && mRectSelection.containsPoint( pt ) ) {
				// Compute the index of the clicked item

				int nClicked;
				if( mTextHeight )
					nClicked = mScrollBar.getTrackPos() + ( pt.y - mRectText.top ) / mTextHeight;
				else
					nClicked = -1;

				// Only proceed if the click falls on top of an item.

				if( nClicked >= mScrollBar.getTrackPos() &&
					nClicked < (int)mItems.size() &&
					nClicked < mScrollBar.getTrackPos() + mScrollBar.getPageSize() )
				{
					SetCapture( CD3DDevice::getInstance().getDeviceWindow() );
					mDragging = true;

					// If this is a double click, fire off an event and exit
					// since the first click would have taken care of the selection
					// updating.
					if( msg == WM_LBUTTONDBLCLK )
					{
						mDialog->sendEvent( UIEVENT_LISTBOX_ITEM_DBLCLK, true, this );
						return true;
					}

					mSelected = nClicked;
					if( !( wParam & MK_SHIFT ) )
						mSelStart = mSelected;

					// If this is a multi-selection listbox, update per-item
					// selection data.

					if( mStyle & MULTISELECTION )
					{
						// Determine behavior based on the state of Shift and Ctrl

						SUIListItem *pSelItem = mItems[ mSelected ];
						if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == MK_CONTROL )
						{
							// Control click. Reverse the selection of this item.

							pSelItem->selected = !pSelItem->selected;
						} else
						if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == MK_SHIFT )
						{
							// Shift click. set the selection for all items
							// from last selected item to the current item.
							// clear everything else.

							int nBegin = min( mSelStart, mSelected );
							int end = max( mSelStart, mSelected );

							int i;

							for( i = 0; i < nBegin; ++i )
							{
								SUIListItem *item = mItems[ i ];
								item->selected = false;
							}

							for( i = end + 1; i < (int)mItems.size(); ++i )
							{
								SUIListItem *item = mItems[ i ];
								item->selected = false;
							}

							for( i = nBegin; i <= end; ++i )
							{
								SUIListItem *item = mItems[ i ];
								item->selected = true;
							}
						} else
						if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == ( MK_SHIFT|MK_CONTROL ) )
						{
							// Control-Shift-click.

							// The behavior is:
							//	 set all items from mSelStart to mSelected to
							//	   the same state as mSelStart, not including mSelected.
							//	 set mSelected to selected.

							int nBegin = min( mSelStart, mSelected );
							int end = max( mSelStart, mSelected );

							// The two ends do not need to be set here.

							bool bLastSelected = mItems[ mSelStart ]->selected;
							for( int i = nBegin + 1; i < end; ++i )
							{
								SUIListItem *item = mItems[ i ];
								item->selected = bLastSelected;
							}

							pSelItem->selected = true;

							// Restore mSelected to the previous value
							// This matches the Windows behavior

							mSelected = mSelStart;
						} else
						{
							// Simple click.  clear all items and select the clicked
							// item.


							for( int i = 0; i < (int)mItems.size(); ++i )
							{
								SUIListItem *item = mItems[ i ];
								item->selected = false;
							}

							pSelItem->selected = true;
						}
					}  // End of multi-selection case

					mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
				}

				return true;
			}
			break;

		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			mDragging = false;

			if( mSelected != -1 )
			{
				// set all items between mSelStart and mSelected to
				// the same state as mSelStart
				int end = max( mSelStart, mSelected );

				for( int n = min( mSelStart, mSelected ) + 1; n < end; ++n )
					mItems[n]->selected = mItems[mSelStart]->selected;
				mItems[mSelected]->selected = mItems[mSelStart]->selected;

				// If mSelStart and mSelected are not the same,
				// the user has dragged the mouse to make a selection.
				// Notify the application of this.
				if( mSelStart != mSelected )
					mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
			}
			return false;
		}

		case WM_MOUSEMOVE:
			if( mDragging )
			{
				// Compute the index of the item below cursor

				int nItem;
				if( mTextHeight )
					nItem = mScrollBar.getTrackPos() + ( pt.y - mRectText.top ) / mTextHeight;
				else
					nItem = -1;

				// Only proceed if the cursor is on top of an item.

				if( nItem >= (int)mScrollBar.getTrackPos() &&
					nItem < (int)mItems.size() &&
					nItem < mScrollBar.getTrackPos() + mScrollBar.getPageSize() )
				{
					mSelected = nItem;
					mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
				} else
				if( nItem < (int)mScrollBar.getTrackPos() )
				{
					// User drags the mouse above window top
					mScrollBar.scroll( -1 );
					mSelected = mScrollBar.getTrackPos();
					mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
				} else
				if( nItem >= mScrollBar.getTrackPos() + mScrollBar.getPageSize() )
				{
					// User drags the mouse below window bottom
					mScrollBar.scroll( 1 );
					mSelected = min( (int)mItems.size(), mScrollBar.getTrackPos() + mScrollBar.getPageSize() ) - 1;
					mDialog->sendEvent( UIEVENT_LISTBOX_SELECTION, true, this );
				}
			}
			break;

		case WM_MOUSEWHEEL:
		{
			UINT lines;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &lines, 0 );
			int nScrollAmount = int((short)HIWORD(wParam)) / WHEEL_DELTA * lines;
			mScrollBar.scroll( -nScrollAmount );
			return true;
		}
	}

	return false;
}


//--------------------------------------------------------------------------------------
void CUIListBox::render( IDirect3DDevice9* device, float dt )
{
	if( !mVisible )
		return;

	SUIElement* element = mElements[ 0 ];
	element->colorTexture.blend( UISTATE_NORMAL, dt );
	element->colorFont.blend( UISTATE_NORMAL, dt );

	SUIElement* selElement = mElements[ 1 ];
	selElement->colorTexture.blend( UISTATE_NORMAL, dt );
	selElement->colorFont.blend( UISTATE_NORMAL, dt );

	mDialog->drawSprite( element, &mBBox );

	// render the text
	if( mItems.size() > 0 ) {
		// Find out the height of a single line of text
		SFRect rc = mRectText;
		SFRect rcSel = mRectSelection;
		rc.bottom = rc.top + CUIResourceManager::getInstance().getFontNode( element->fontIdx )->height;

		// Update the line height formation
		mTextHeight = rc.bottom - rc.top;

		static bool sbInit;
		if( !sbInit ) {
			// Update the page size of the scroll bar
			if( mTextHeight )
				mScrollBar.setPageSize( mRectText.getHeight() / mTextHeight );
			else
				mScrollBar.setPageSize( mRectText.getHeight() );
			sbInit = true;
		}

		rc.right = mRectText.right;
		for( int i = mScrollBar.getTrackPos(); i < (int)mItems.size(); ++i )
		{
			if( rc.bottom > mRectText.bottom )
				break;

			SUIListItem *item = mItems[ i ];

			// Determine if we need to render this item with the
			// selected element.
			bool bSelectedStyle = false;

			if( !( mStyle & MULTISELECTION ) && i == mSelected )
				bSelectedStyle = true;
			else
			if( mStyle & MULTISELECTION )
			{
				if( mDragging &&
					( ( i >= mSelected && i < mSelStart ) ||
					  ( i <= mSelected && i > mSelStart ) ) )
					bSelectedStyle = mItems[mSelStart]->selected;
				else
				if( item->selected )
					bSelectedStyle = true;
			}
			
			if( bSelectedStyle )
			{
				rcSel.top = rc.top; rcSel.bottom = rc.bottom;
				mDialog->drawSprite( selElement, &rcSel );
				mDialog->drawText( item->text, selElement, &rc, !selElement->darkFont );
			}
			else
				mDialog->drawText( item->text, element, &rc, !element->darkFont );

			rc.offset( 0, mTextHeight );
		}
	}

	// render the scroll bar

	mScrollBar.render( device, dt );
}




// --------------------------------------------------------------------------

void SBlendColor::init( bool font, bool darkFont )
{
	D3DCOLOR cnormal = font ? (darkFont ? colTDNormal : colTLNormal) : colINormal;
	for( int i=0; i < UI_MAX_CTRL_STATES; ++i )
		colors[ i ] = cnormal;
	if( font ) {
		colors[ UISTATE_PRESSED		] = darkFont ? colTDPress : colTLPress;
		colors[ UISTATE_MOUSEOVER	] = darkFont ? colTDHover : colTLHover;
		colors[ UISTATE_FOCUS		] = darkFont ? colTDFocus : colTLFocus;
		colors[ UISTATE_DISABLED	] = darkFont ? colTDDisable : colTLDisable;
	} else {
		colors[ UISTATE_PRESSED		] = colIPressed;
		colors[ UISTATE_MOUSEOVER	] = colIHover;
		colors[ UISTATE_FOCUS		] = colIFocus;
		colors[ UISTATE_DISABLED	] = colIDisable;
	}
	colors[ UISTATE_HIDDEN ] = colHidden;
	current = colHidden;
}

void SBlendColor::blend( UINT state, float dt, float rate )
{
	D3DXCOLOR destColor = colors[ state ];
	D3DXColorLerp( &current, &current, &destColor, 1.0f - powf( rate, 30 * dt ) );
}



// --------------------------------------------------------------------------

void SUIElement::setTexture( CD3DTexture* tex, const RECT* texRect )
{
	texture = tex;
	if( texRect )
		textureRect = *texRect;
	else
		SetRectEmpty( &textureRect );
	colorTexture.init( false );
}

void SUIElement::setFont( UINT fIdx, bool dark, DWORD textFmt )
{
	fontIdx = fIdx;
	textFormat = textFmt;
	darkFont = dark;
	colorFont.init( true, darkFont );
}

void SUIElement::refresh()
{
	colorTexture.current = colorTexture.colors[ UISTATE_HIDDEN ];
	colorFont.current = colorFont.colors[ UISTATE_HIDDEN ];
}
