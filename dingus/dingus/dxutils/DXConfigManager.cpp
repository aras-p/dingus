#include "stdafx.h"
#include <stdio.h>
#include "DXConfigDB.h"
#include "DXConfigManager.h"

using namespace dingus;


typedef struct
{
	const char* Property;
	bool (CConfigManager::*Func)(const char*);
} CONFIGTABLE;

CONFIGTABLE ConfigTable[]= {
	{ "ForceShader",					  &CConfigManager::InitForceShader						},
	{ "DisableDriverManagement",		  &CConfigManager::InitDisableDriverManagement			},
	{ "LinearTextureAddressing",		  &CConfigManager::InitLinearTextureAddressing			},
	{ "MaximumResolution",				  &CConfigManager::InitMaximumResolution				},
	{ "UnsupportedCard",				  &CConfigManager::InitUnsupportedCard					},
	{ "OldDriver",						  &CConfigManager::InitOldDriver						},
	{ "EnableStopStart",				  &CConfigManager::InitEnableStopStart					},
	{ "InvalidDriver",					  &CConfigManager::InitInvalidDriver					},
	{ "UseAnisotropicFilter",			  &CConfigManager::InitUseAnisotropicFilter 			},
	{ "UMA",							  &CConfigManager::InitUMA								},
	{ "DoNotUseMinMaxBlendOp",			  &CConfigManager::InitDoNotUseMinMaxBlendOp			},
};


static DWORD GetNumber( const char* value )
{
	int result;
	int nRet = sscanf( value, "%d", &result );
	if( nRet == 0 || nRet == EOF )
		result = 0;
	return result;
}


static float GetFloatNumber( const char* value )
{
	float result;
	int nRet = sscanf( value, "%f", &result );
	if( nRet == 0 || nRet == EOF )
		result = 0.0f;
	return result;
}



CConfigManager::CConfigManager()
{
	InitConfigProperties();
}


void CConfigManager::InitConfigProperties()
{
	cf_MaximumResolution = 4096;
	cf_LinearTextureAddressing = 0;
	cf_UseAnisotropicFilter = 0;
	cf_UnsupportedCard = 0;
	cf_EnableStopStart = 0;
	cf_OldDriver = 0;
	cf_InvalidDriver = 0;
	cf_ForceShader = 0;
	cf_DoNotUseMinMaxBlendOp = 0;
}



HRESULT CConfigManager::Initialize( WCHAR* FileName, const CD3DDeviceCaps& caps )
{
	int i;

	char buf[1000];
	HRESULT hr = S_OK;
	
	
	IConfigDatabase* pCDB = IConfigDatabase::Create();
	if( pCDB->Load( FileName, caps, 128 ) ) {
		sprintf( buf, "Graphics device string: %s\n", pCDB->GetGfxDeviceString() );
		OutputDebugString( buf );
		sprintf( buf, "Graphics vendor string: %s\n", pCDB->GetGfxVendorString() );
		OutputDebugString( buf );
		
		// Interate the properties and initialize the config flags.
		OutputDebugString( "Properties:\n" );
		int nPropCount = pCDB->GetDevicePropertyCount();
		for( i = 0; i < nPropCount; ++i ) {
			sprintf( buf, "\"%s\" : \"%s\"\n", pCDB->GetDeviceProperty( i ), pCDB->GetDeviceValue( i ) );
			OutputDebugString( buf );
			
			for( int f = 0; f < sizeof(ConfigTable) / sizeof(ConfigTable[0]); ++f ) {
				if( !_stricmp( ConfigTable[f].Property, pCDB->GetDeviceProperty( i ) ) ) {
					// Property found. Call the init function.
					(this->*ConfigTable[f].Func)( pCDB->GetDeviceValue( i ) );
				}
			}
		}
		
		OutputDebugString( "\nRequirements:\n" );
		int nReqCount = pCDB->GetRequirementsPropertyCount();
		for( i = 0; i < nReqCount; ++i ) {
			char* Property = (char*)pCDB->GetRequirementsProperty( i );
			char* Value = (char*)pCDB->GetRequirementsValue( i );
			int nRet;
			
			sprintf( buf, "\"%s\" : \"%s\"\n", Property, Value );
			OutputDebugString( buf );
			
			if( !_stricmp( "CpuSpeed", Property ) )
			{
				nRet = sscanf( Value, "%d",&req_CPUSpeed );
				if( nRet == 0 || nRet == EOF )
					req_CPUSpeed = 0;
			} else
				if( !_stricmp( "Memory", Property ) )
				{
					nRet = sscanf( Value, "%d",&req_Memory );
					if( nRet == 0 || nRet == EOF )
						req_Memory = 0;
				} else
					if( !_stricmp( "VideoMemory", Property ) )
					{
						nRet = sscanf( Value, "%d",&req_VideoMemory );
						if( nRet == 0 || nRet == EOF )
							req_VideoMemory = 0;
					} else
						if( !_stricmp( "DirectX", Property ) )
						{
							nRet = sscanf( Value, "%d.%d%c", &req_DirectXMajor, &req_DirectXMinor, &req_DirectXLetter );
							req_DirectXLetter = (char)req_DirectXLetter;  // Convert to wide char
							if( nRet == 0 || nRet == EOF )
							{
								req_DirectXMajor = req_DirectXMinor = 0;
								req_DirectXLetter = L' ';
							}
						} else
							if( !_stricmp( "DiskSpace", Property ) )
							{
								nRet = sscanf( Value, "%d",&req_DiskSpace );
								if( nRet == 0 || nRet == EOF )
									req_DiskSpace = 0;
							} else
								if( !_stricmp( "OS", Property ) )
								{
									if( !_stricmp( "Win95", Value ) )
									{
										req_OSMajor = 4;
										req_OSMinor = 0;
									} else
										if( !_stricmp( "Win98", Value ) )
										{
											req_OSMajor = 4;
											req_OSMinor = 10;
										} else
											if( !_stricmp( "WinME", Value ) )
											{
												req_OSMajor = 4;
												req_OSMinor = 90;
											} else
												if( !_stricmp( "Win2k", Value ) )
												{
													req_OSMajor = 5;
													req_OSMinor = 0;
												} else
													if( !_stricmp( "WinXP", Value ) )
													{
														req_OSMajor = 5;
														req_OSMinor = 1;
													} else
														if( !_stricmp( "Win2003", Value ) )
														{
															req_OSMajor = 5;
															req_OSMinor = 2;
														}
								}
		}
	}
	else
	{
		hr = E_FAIL;
	}
	
	pCDB->Release();
	
	return hr;
}


bool CConfigManager::InitMaximumResolution( const char* value )
{
	DWORD val = GetNumber( value );
	if( val < 640 || val > 4096 )
	{
		return false;
	}
	cf_MaximumResolution=val;
	return true;
}


bool CConfigManager::InitForceShader( const char* value )
{
	if( *(WORD*)value=='a2' || *(WORD*)value=='A2' )
	{
		cf_ForceShader=9998;
	}
	else
		if( *(WORD*)value=='b2' || *(WORD*)value=='B2' )
		{
			cf_ForceShader=9997;
		}
		else
		{
			cf_ForceShader = GetNumber( value );
			if( 0==cf_ForceShader )
				cf_ForceShader=9999;
		}
		return true;
}


bool CConfigManager::InitLinearTextureAddressing( const char* value )
{
	cf_LinearTextureAddressing=1;
	return true;
}


bool CConfigManager::InitUseAnisotropicFilter( const char* value )
{
	cf_UseAnisotropicFilter=1;
	return true;
}


bool CConfigManager::InitUnsupportedCard( const char* value )
{
	cf_UnsupportedCard=1;
	return true;
}


bool CConfigManager::InitOldDriver( const char* value )
{
	cf_OldDriver=1;
	return true;
}


bool CConfigManager::InitEnableStopStart( const char* value )
{
	cf_EnableStopStart=1;
	return true;
}

bool CConfigManager::InitInvalidDriver( const char* value )
{
	cf_InvalidDriver=1;
	return true;
}



//
//	 0-64Meg  = 8Meg video memory
//	64-128Meg = 16Meg video memory
// 128-256	  = 32Meg video memory
// 256+ 	  = 64Meg video memory
//
bool CConfigManager::InitUMA( const char* value )
{
	/*
	VideoMemory = 8*1024*1024;
	if( SysMemory >= 64 )
		VideoMemory = 16*1024*1024;
	if( SysMemory >= 128 )
		VideoMemory = 32*1024*1024;
	if( SysMemory >= 256 )
		VideoMemory = 64*1024*1024;
	*/
	return true;
}


bool CConfigManager::InitDoNotUseMinMaxBlendOp( const char* value )
{
	cf_DoNotUseMinMaxBlendOp = 1;
	return true;
}

/*
HRESULT CConfigManager::VerifyRequirements()
{
	HRESULT hr = S_OK;
	WCHAR wszText[2048] = L"This system does not meet the following minimum requirement(s):\n\n";
	
	//
	// OS version
	//
	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx( &ovi );
	if( req_OSMajor > ovi.dwMajorVersion ||
		( req_OSMajor == ovi.dwMajorVersion && req_OSMinor > ovi.dwMinorVersion ) )
	{
		// OS version not supported.
		hr = E_FAIL;
		wcsncat( wszText, L"- Operating system not supported\n", 2048 - lstrlenW( wszText ) );
		wszText[2047] = L'\0';
	}
	
	//
	// CPU speed
	//
	if( CpuSpeed < req_CPUSpeed )
	{
		hr = E_FAIL;
		WCHAR wsz[256];
		swprintf( wsz, L"- Minimum processor speed of %u MHz\n", req_CPUSpeed );
		wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
		wszText[2047] = L'\0';
	}
	
	//
	// Memory size
	//
	if( SysMemory < req_Memory )
	{
		hr = E_FAIL;
		WCHAR wsz[256];
		swprintf( wsz, L"- Minimum memory of %u megabytes\n", req_Memory );
		wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
		wszText[2047] = L'\0';
	}
	
	//
	// Video memory size
	//
	if( (VideoMemory >> 20) < req_VideoMemory )
	{
		hr = E_FAIL;
		WCHAR wsz[256];
		swprintf( wsz, L"- Minimum video memory of %u megabytes\n", req_VideoMemory );
		wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
		wszText[2047] = L'\0';
	}
	
	//
	// DirectX version
	//
	DWORD dwDXMajor = 0;
	DWORD dwDXMinor = 0;
	TCHAR cDXLetter = ' ';
	GetDXVersion( &dwDXMajor, &dwDXMinor, &cDXLetter );
	
	if( dwDXMajor < req_DirectXMajor ||
		( dwDXMajor == req_DirectXMajor && dwDXMinor < req_DirectXMinor ) ||
		( dwDXMajor == req_DirectXMajor && dwDXMinor == req_DirectXMinor && cDXLetter < req_DirectXLetter ) )
	{
		hr = E_FAIL;
		WCHAR wsz[256];
		swprintf( wsz, L"- DirectX %u.%u%c\n", req_DirectXMajor, req_DirectXMinor, req_DirectXLetter );
		wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
		wszText[2047] = L'\0';
	}
	
	if( FAILED( hr ) )
	{
		::MessageBox( NULL, wszText, L"Minimum Requirement", MB_OK|MB_ICONERROR );
	}
	
	return hr;
}
*/