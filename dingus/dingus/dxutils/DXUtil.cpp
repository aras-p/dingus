// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "DXUtil.h"

#include <mmsystem.h>

using namespace dingus;


/*

double __stdcall dingus::timer( eTimerCommand command )
{
	static bool 	mTimerInited	= false;
	static bool 	mUsingQPF		= false;
	static bool 	mTimerStopped	= true;
	static LONGLONG mQPFTicksPerSec = 0;
	
	// initialize the timer
	if( !mTimerInited ) {
		mTimerInited = true;
		
		// Use QueryPerformanceFrequency() to get frequency of timer. If QPF
		// is not supported, we will timeGetTime() which returns milliseconds.
		LARGE_INTEGER ticksPerSec;
		mUsingQPF = QueryPerformanceFrequency( &ticksPerSec ) ? true : false;
		if( mUsingQPF )
			mQPFTicksPerSec = ticksPerSec.QuadPart;
	}

	
	if( mUsingQPF ) {

		//
		// QPF

		static LONGLONG mStopTime			= 0;
		static LONGLONG mLastElapsedTime	= 0;
		static LONGLONG mBaseTime			= 0;
		double time;
		double elapsedTime;
		LARGE_INTEGER qwTime;
		
		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		if( mStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME )
			qwTime.QuadPart = mStopTime;
		else
			QueryPerformanceCounter( &qwTime );
		
		// Return the elapsed time
		if( command == TIMER_GETELAPSEDTIME ) {
			elapsedTime = (double)(qwTime.QuadPart - mLastElapsedTime) / (double)mQPFTicksPerSec;
			mLastElapsedTime = qwTime.QuadPart;
			return elapsedTime;
		}
		
		// Return the current time
		if( command == TIMER_GETAPPTIME ) {
			double appTime = (double)(qwTime.QuadPart - mBaseTime) / (double)mQPFTicksPerSec;
			return appTime;
		}
		
		// Reset the timer
		if( command == TIMER_RESET ) {
			mBaseTime			= qwTime.QuadPart;
			mLastElapsedTime	= qwTime.QuadPart;
			mStopTime			= 0;
			mTimerStopped		= false;
			return 0.0;
		}
		
		// Start the timer
		if( command == TIMER_START ) {
			if( mTimerStopped )
				mBaseTime += qwTime.QuadPart - mStopTime;
			mStopTime = 0;
			mLastElapsedTime = qwTime.QuadPart;
			mTimerStopped = false;
			return 0.0;
		}
		
		// Stop the timer
		if( command == TIMER_STOP ) {
			if( !mTimerStopped ) {
				mStopTime = qwTime.QuadPart;
				mLastElapsedTime = qwTime.QuadPart;
				mTimerStopped = true;
			}
			return 0.0;
		}
		
		// Advance the timer by 1/10th second
		if( command == TIMER_ADVANCE ) {
			mStopTime += mQPFTicksPerSec/10;
			return 0.0f;
		}
		
		if( command == TIMER_GETABSOLUTETIME ) {
			time = qwTime.QuadPart / (double)mQPFTicksPerSec;
			return time;
		}
		
		return -1.0; // Invalid command specified

	} else {

		//
		// Get the time using timeGetTime()

		static double mLastElapsedTime = 0.0;
		static double mBaseTime = 0.0;
		static double mStopTime	= 0.0;
		double time;
		double elapsedTime;
		
		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		if( mStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
			time = mStopTime;
		else
			time = timeGetTime() * 0.001;
		
		// Return the elapsed time
		if( command == TIMER_GETELAPSEDTIME ) {	
			elapsedTime = (double)(time - mLastElapsedTime);
			mLastElapsedTime = time;
			return elapsedTime;
		}
		
		// Return the current time
		if( command == TIMER_GETAPPTIME ) {
			return (time - mBaseTime);
		}
		
		// Reset the timer
		if( command == TIMER_RESET ) {
			mBaseTime 			= time;
			mLastElapsedTime	= time;
			mStopTime 			= 0;
			mTimerStopped	  = false;
			return 0.0;
		}
		
		// Start the timer
		if( command == TIMER_START ) {
			if( mTimerStopped )
				mBaseTime += time - mStopTime;
			mStopTime = 0.0;
			mLastElapsedTime	= time;
			mTimerStopped = false;
			return 0.0;
		}
		
		// Stop the timer
		if( command == TIMER_STOP ) {
			if( !mTimerStopped ) {
				mStopTime = time;
				mLastElapsedTime = time;
				mTimerStopped = true;
			}
			return 0.0;
		}
		
		// Advance the timer by 1/10th second
		if( command == TIMER_ADVANCE ) {
			mStopTime += 0.1;
			return 0.0;
		}
		
		if( command == TIMER_GETABSOLUTETIME ) {
			return time;
		}
		
		return -1.0; // Invalid command specified
	}
}

*/

//-----------------------------------------------------------------------------
// Name: convertAnsiStringToWideCch()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT dingus::convertAnsiStringToWideCch( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                                       wstrDestination, cchDestChar );
    wstrDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: convertWideStringToAnsi()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       CHAR string. 
//       cchDestChar is the size in TCHARs of strDestination
//-----------------------------------------------------------------------------
HRESULT dingus::convertWideStringToAnsiCch( CHAR* strDestination, const WCHAR* wstrSource, 
                                     int cchDestChar )
{
    if( strDestination==NULL || wstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = WideCharToMultiByte( CP_ACP, 0, wstrSource, -1, strDestination, 
                                       cchDestChar*sizeof(CHAR), NULL, NULL );
    strDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: convertGenericStringToAnsi()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       CHAR string. 
//       cchDestChar is the size in TCHARs of strDestination
//-----------------------------------------------------------------------------
HRESULT dingus::convertGenericStringToAnsiCch( CHAR* strDestination, const TCHAR* tstrSource, 
                                           int cchDestChar )
{
    if( strDestination==NULL || tstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    return convertWideStringToAnsiCch( strDestination, tstrSource, cchDestChar );
#else
    strncpy( strDestination, tstrSource, cchDestChar );
    strDestination[cchDestChar-1] = '\0';
    return S_OK;
#endif   
}




//-----------------------------------------------------------------------------
// Name: convertGenericStringToWide()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT dingus::convertGenericStringToWideCch( WCHAR* wstrDestination, const TCHAR* tstrSource, 
                                           int cchDestChar )
{
    if( wstrDestination==NULL || tstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    wcsncpy( wstrDestination, tstrSource, cchDestChar );
    wstrDestination[cchDestChar-1] = L'\0';
    return S_OK;
#else
    return convertAnsiStringToWideCch( wstrDestination, tstrSource, cchDestChar );
#endif    
}




//-----------------------------------------------------------------------------
// Name: convertAnsiStringToGeneric()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT dingus::convertAnsiStringToGenericCch( TCHAR* tstrDestination, const CHAR* strSource, 
                                           int cchDestChar )
{
    if( tstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;
        
#ifdef _UNICODE
    return convertAnsiStringToWideCch( tstrDestination, strSource, cchDestChar );
#else
    strncpy( tstrDestination, strSource, cchDestChar );
    tstrDestination[cchDestChar-1] = '\0';
    return S_OK;
#endif    
}




//-----------------------------------------------------------------------------
// Name: convertAnsiStringToGeneric()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT dingus::convertWideStringToGenericCch( TCHAR* tstrDestination, const WCHAR* wstrSource, 
                                           int cchDestChar )
{
    if( tstrDestination==NULL || wstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    wcsncpy( tstrDestination, wstrSource, cchDestChar );
    tstrDestination[cchDestChar-1] = L'\0';    
    return S_OK;
#else
    return convertWideStringToAnsiCch( tstrDestination, wstrSource, cchDestChar );
#endif
}





//-----------------------------------------------------------------------------
// Name: BYTE helper functions
// Desc: cchDestChar is the size in BYTEs of strDest.  Be careful not to 
//       pass use sizeof() if the strDest is a string pointer.  
//       eg.
//       TCHAR* sz = new TCHAR[100]; // sizeof(sz)  == 4
//       TCHAR sz2[100];             // sizeof(sz2) == 200
//-----------------------------------------------------------------------------
HRESULT convertAnsiStringToWideCb( WCHAR* wstrDestination, const CHAR* strSource, int cbDestChar )
{
    return convertAnsiStringToWideCch( wstrDestination, strSource, cbDestChar / sizeof(WCHAR) );
}

HRESULT convertWideStringToAnsiCb( CHAR* strDestination, const WCHAR* wstrSource, int cbDestChar )
{
    return convertWideStringToAnsiCch( strDestination, wstrSource, cbDestChar / sizeof(CHAR) );
}

HRESULT convertGenericStringToAnsiCb( CHAR* strDestination, const TCHAR* tstrSource, int cbDestChar )
{
    return convertGenericStringToAnsiCch( strDestination, tstrSource, cbDestChar / sizeof(CHAR) );
}

HRESULT convertGenericStringToWideCb( WCHAR* wstrDestination, const TCHAR* tstrSource, int cbDestChar )
{
    return convertGenericStringToWideCch( wstrDestination, tstrSource, cbDestChar / sizeof(WCHAR) );
}

HRESULT convertAnsiStringToGenericCb( TCHAR* tstrDestination, const CHAR* strSource, int cbDestChar )
{
    return convertAnsiStringToGenericCch( tstrDestination, strSource, cbDestChar / sizeof(TCHAR) );
}

HRESULT convertWideStringToGenericCb( TCHAR* tstrDestination, const WCHAR* wstrSource, int cbDestChar )
{
    return convertWideStringToGenericCch( tstrDestination, wstrSource, cbDestChar / sizeof(TCHAR) );
}
