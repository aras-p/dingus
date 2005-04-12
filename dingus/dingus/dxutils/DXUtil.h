// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef DX_UTIL_H__
#define DX_UTIL_H__


namespace dingus {

//---------------------------------------------------------------------------

/**
 *  Performs timer opertations.
 *
 *  Use the following commands:
 *		TIMER_RESET           - to reset the timer
 *		TIMER_START           - to start the timer
 *		TIMER_STOP            - to stop (or pause) the timer
 *		TIMER_ADVANCE         - to advance the timer by 0.1 seconds
 *		TIMER_GETABSOLUTETIME - to get the absolute system time
 *		TIMER_GETAPPTIME      - to get the current time
 *		TIMER_GETELAPSEDTIME  - to get the time that elapsed between TIMER_GETELAPSEDTIME calls
 */
//enum eTimerCommand {
//	TIMER_RESET, TIMER_START, TIMER_STOP, TIMER_ADVANCE,
//	TIMER_GETABSOLUTETIME, TIMER_GETAPPTIME, TIMER_GETELAPSEDTIME
//};

//double __stdcall timer( eTimerCommand command );


//---------------------------------------------------------------------------
// UNICODE support for converting between CHAR, TCHAR, and WCHAR strings

HRESULT convertAnsiStringToWideCch( WCHAR* wstrDestination, const CHAR* strSource, int cchDestChar );
HRESULT convertWideStringToAnsiCch( CHAR* strDestination, const WCHAR* wstrSource, int cchDestChar );
HRESULT convertGenericStringToAnsiCch( CHAR* strDestination, const TCHAR* tstrSource, int cchDestChar );
HRESULT convertGenericStringToWideCch( WCHAR* wstrDestination, const TCHAR* tstrSource, int cchDestChar );
HRESULT convertAnsiStringToGenericCch( TCHAR* tstrDestination, const CHAR* strSource, int cchDestChar );
HRESULT convertWideStringToGenericCch( TCHAR* tstrDestination, const WCHAR* wstrSource, int cchDestChar );
HRESULT convertAnsiStringToWideCb( WCHAR* wstrDestination, const CHAR* strSource, int cbDestChar );
HRESULT convertWideStringToAnsiCb( CHAR* strDestination, const WCHAR* wstrSource, int cbDestChar );
HRESULT convertGenericStringToAnsiCb( CHAR* strDestination, const TCHAR* tstrSource, int cbDestChar );
HRESULT convertGenericStringToWideCb( WCHAR* wstrDestination, const TCHAR* tstrSource, int cbDestChar );
HRESULT convertAnsiStringToGenericCb( TCHAR* tstrDestination, const CHAR* strSource, int cbDestChar );
HRESULT convertWideStringToGenericCb( TCHAR* tstrDestination, const WCHAR* wstrSource, int cbDestChar );



}; // namespace dingus


#endif
