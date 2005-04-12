// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "InputEvent.h"

using namespace dingus;


//---------------------------------------------------------------------------
// CInputEvent
//---------------------------------------------------------------------------

int CInputEvent::unique( const CInputEvent& event )
{
	return event.getType();
}

bool CInputEvent::operator< ( CInputEvent const& rhs ) const
{
	return(	unique(*this) < unique( rhs ) );
}


//---------------------------------------------------------------------------
// CKeyEvent
//---------------------------------------------------------------------------

int CKeyEvent::unique( const CKeyEvent& event, bool withSpecialKey )
{
	return
		(event.getMode()<<30) +
		(event.getAscii()<<16) +
		(withSpecialKey ? (event.getKeyCode()) : 0);
}

bool CKeyEvent::operator< ( CInputEvent const& rhs ) const
{
	if( CInputEvent::operator<( rhs ) )
		return true;
	if( getType() != rhs.getType() )
		return false;
	const CKeyEvent& k = (CKeyEvent const&)rhs;
	if( !mKeyCode || !k.mKeyCode )
		return unique(*this,false) < unique(k,false);
	else
		return unique(*this,true) < unique(k,true);
}


//---------------------------------------------------------------------------
// CMouseEvent
//---------------------------------------------------------------------------

int CMouseEvent::unique( const CMouseEvent& event )
{
	return
		(event.getMode() << 20 ) +
		(event.getButton() << 16 ) +		
		(event.getClickCount());
}

bool CMouseEvent::operator< ( CInputEvent const& rhs ) const
{
	if( CInputEvent::operator<( rhs ) )
		return true;
	if( getType() != rhs.getType() )
		return false;
	return(	unique( *this ) < unique( (CMouseEvent const&)rhs ) );
}
