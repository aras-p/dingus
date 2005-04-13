#include "stdafx.h"
#include "CpuTimer.h"
#include "../console/Console.h"


namespace dingus {
namespace cputimer {
	static int		sCpuMhz = 0;
	static double	sSecsPerCpuTick = 0.0;
	
	void computeMhz();
};
};


#pragma warning(disable:4035)


dingus::cputimer::ticks_type	dingus::cputimer::ticks()
{
	__asm rdtsc;
	// eax/edx returned
}

int	dingus::cputimer::mhz()
{
	if ( sCpuMhz == 0 )
	{
		computeMhz();
	}
	return sCpuMhz;
}

double dingus::cputimer::secsPerTick()
{
	if ( sCpuMhz == 0 )
	{
		computeMhz();
	}
	return sSecsPerCpuTick;	
}

double dingus::cputimer::seconds()
{
	ticks_type t = ticks();
	return double(t) * sSecsPerCpuTick;
}

#define TSC_PATH	"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
#define TSC_FILE	 "~MHz"

static bool registryGetValue(const char * path,const char * file,int * pValue)
{
	HKEY hKey;
	DWORD type;
	DWORD size;

	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,path,0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS )
		return false;

	bool ok = ( RegQueryValueEx( hKey, file, 0, &type, (BYTE*)pValue, &size ) == ERROR_SUCCESS );
	RegCloseKey (hKey);

	return ok;
}

void dingus::cputimer::computeMhz()
{
	if( !registryGetValue(TSC_PATH,TSC_FILE,&sCpuMhz) ) {
		// can't get to the registry value !
		// calibrate myself
		// just do a shit calibration:

		// try a max of 4 times
		for( int tries = 0; tries < 4; ++tries ) {
			// first busy loop to align to a ms
			DWORD tick1 = GetTickCount();
			while ( GetTickCount() == tick1 ) ;
			
			// then measure the # of ticks for one more ms to pass :
			ticks_type tsc1 = ticks();
			DWORD tick2 = GetTickCount();
			while ( GetTickCount() == tick2 ) ;
			ticks_type tsc2 = ticks();	
			
			// 1 ms has passed
			ticks_type deltaTsc = tsc2 - tsc1;
			DWORD hz = ((DWORD)deltaTsc) * 1000;
			// round to the nearest million :
			sCpuMhz = (hz + 500000)/1000000;
			
			if ( sCpuMhz > 100 )
			{
				// it's good, get out
				break;
			}
		}
	}
	
	sSecsPerCpuTick = 1.0 / (sCpuMhz * 1000000.0);
}	

// --------------------------------------------------------------------------

dingus::cputimer::debug_interval::debug_interval( const char* name )
: mStartTicks( ticks() )
, mName(name)
{
}

dingus::cputimer::debug_interval::~debug_interval()
{
	ticks_type nowTicks = ticks();
	double secs = (nowTicks - mStartTicks) * secsPerTick();

	char buf[100];
	sprintf( buf, "%s: %.6f", mName, secs );
	CConsole::getChannel("cputimer").write( buf );
}

