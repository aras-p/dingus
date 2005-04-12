#ifndef __DX_CONFIGMANAGER_H
#define __DX_CONFIGMANAGER_H

HRESULT GetDXVersion( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, WCHAR* pcDirectXVersionLetter );

namespace dingus {
	
class CD3DDeviceCaps;

	
//--------------------------------------------------------------------------------------
// Class CConfigManager encapsulates configuration flags and methods for loading
// these flags.
//--------------------------------------------------------------------------------------
class CConfigManager
{
public:
	// Configuration Flags
	// These flags are initialized from device detection result and the configuration
	// database file.
	
	DWORD cf_ForceShader;
	DWORD cf_MaximumResolution;
	DWORD cf_LinearTextureAddressing;
	DWORD cf_UnsupportedCard;
	DWORD cf_OldDriver;
	DWORD cf_InvalidDriver;
	DWORD cf_UseAnisotropicFilter;
	DWORD cf_DoNotUseMinMaxBlendOp;
	DWORD cf_EnableStopStart;

	char* VendorName;
	char* DeviceName;
	DWORD DeviceID;
	DWORD VendorID;
	DWORD DriverVersionLowPart;
	DWORD DriverVersionHighPart;
	char* AppendText;

	DWORD req_CPUSpeed;
	DWORD req_Memory;
	DWORD req_VideoMemory;
	DWORD req_DirectXMajor;
	DWORD req_DirectXMinor;
	int   req_DirectXLetter;
	DWORD req_DiskSpace;
	DWORD req_OSMajor;
	DWORD req_OSMinor;
	
public:
	CConfigManager();
	void InitConfigProperties();
	HRESULT VerifyRequirements();
	HRESULT Initialize( WCHAR* FileName, const CD3DDeviceCaps& caps );

	bool InitMaximumResolution( const char* value );
	bool InitForceShader( const char* value );
	bool InitDisableBuffering( const char* value );
	bool InitLinearTextureAddressing( const char* value );
	bool InitUseAnisotropicFilter( const char* value );
	bool InitUnsupportedCard( const char* value );
	bool InitOldDriver( const char* value );
	bool InitEnableStopStart( const char* value );
	bool InitInvalidDriver( const char* value );
	bool InitDisableDriverManagement( const char* value );
	bool InitUMA( const char* value );
	bool InitDoNotUseMinMaxBlendOp( const char* value );
};
	
	
}; // namespace


#endif
