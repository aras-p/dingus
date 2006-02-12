// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "D3DEnumeration.h"

using namespace dingus;


//---------------------------------------------------------------------------

/**
 *  Returns number of color channel bits for D3DFORMAT.
 */
static int gGetColorChannelBits( D3DFORMAT fmt ) {
	switch( fmt ) {
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A2R10G10B10:
		return 10;
	case D3DFMT_R8G8B8:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		return 8;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
		return 5;
	case D3DFMT_A4R4G4B4:
	case D3DFMT_X4R4G4B4:
		return 4;
	case D3DFMT_R3G3B2:
	case D3DFMT_A8R3G3B2:
		return 2;
	default:
		return 0;
	}
};


//---------------------------------------------------------------------------

/**
 *  Returns number of alpha channel bits for D3DFORMAT.
 */
static int gGetAlphaChannelBits( D3DFORMAT fmt ) {
	switch( fmt ) {
	case D3DFMT_R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_R3G3B2:
	case D3DFMT_X4R4G4B4:
		return 0;
	case D3DFMT_A8R8G8B8:
	case D3DFMT_A8R3G3B2:
		return 8;
	case D3DFMT_A1R5G5B5:
		return 1;
	case D3DFMT_A4R4G4B4:
		return 4;
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A2R10G10B10:
		return 2;
	default:
		return 0;
	}
};


//---------------------------------------------------------------------------

/**
 *  Returns number of depth bits for D3DFORMAT.
 */
static int gGetDepthBits( D3DFORMAT fmt ) {
	switch( fmt ) {
	case D3DFMT_D16:
		return 16;
	case D3DFMT_D15S1:
		return 15;
	case D3DFMT_D24X8:
	case D3DFMT_D24S8:
	case D3DFMT_D24X4S4:
		return 24;
	case D3DFMT_D32:
		return 32;
	default:
		return 0;
	}
};


//---------------------------------------------------------------------------

/**
 *  Returns number of stencil bits for D3DFORMAT.
 */
static int gGetStencilBits( D3DFORMAT fmt ) {
	switch( fmt ) {
	case D3DFMT_D16:
	case D3DFMT_D24X8:
	case D3DFMT_D32:
		return 0;
	case D3DFMT_D15S1:
		return 1;
	case D3DFMT_D24S8:
		return 8;
	case D3DFMT_D24X4S4:
		return 4;
	default:
		return 0;
	}
};


//---------------------------------------------------------------------------
//  SD3DAdapterInfo
//---------------------------------------------------------------------------

SD3DAdapterInfo::~SD3DAdapterInfo()
{
	stl_utils::wipe( deviceInfos );
}

//---------------------------------------------------------------------------
// SD3DDeviceInfo
//---------------------------------------------------------------------------

SD3DDeviceInfo::~SD3DDeviceInfo()
{
	stl_utils::wipe( deviceCombos );
	safeRelease( configDB );
}



//---------------------------------------------------------------------------
// CD3DEnumeration
//---------------------------------------------------------------------------


CD3DEnumeration::CD3DEnumeration()
:	mConfirmDeviceCallback(NULL),
	mConfigDBFileName(NULL),
	mMinFullscreenWidth(640),
	mMinFullscreenHeight(480),
	mMinColorChannelBits(5),
	mMinAlphaChannelBits(0),
	mMinDepthBits(15),
	mMinStencilBits(0),
	mUsesDepthBuffer(false),
	mUsesMixedVP(false),
	mRequiresWindowed(false),
	mRequiresFullscreen(false)
{
}


CD3DEnumeration::~CD3DEnumeration()
{
	stl_utils::wipe( mAdapterInfos );
	stl_utils::wipe( mIncompatAdapterInfos );
}


//-----------------------------------------------------------------------------

// Used to sort display modes.
/*
bool operator< ( const D3DDISPLAYMODE& dm1, const D3DDISPLAYMODE& dm2 )
{
	if (dm1.Width < dm2.Width)
		return true;
	if (dm1.Height < dm2.Height)
		return true;
	if (dm1.Format < dm2.Format)
		return true;
	if (dm1.RefreshRate < dm2.RefreshRate)
		return true;
	return false;
};
*/


HRESULT CD3DEnumeration::enumerate()
{
	HRESULT hr;
	
	if( !mDirect3D )
		return E_FAIL;
	
	stl_utils::wipe( mAdapterInfos );
	stl_utils::wipe( mIncompatAdapterInfos );
	
	mAllowedFormats.clear();
	mAllowedFormats.push_back( D3DFMT_X8R8G8B8 );
	mAllowedFormats.push_back( D3DFMT_X1R5G5B5 );
	mAllowedFormats.push_back( D3DFMT_R5G6B5 );
	mAllowedFormats.push_back( D3DFMT_A2R10G10B10 );

	//
	// get all adapters

	int adapterCount = mDirect3D->GetAdapterCount();
	for( int adOrd = 0; adOrd < adapterCount; ++adOrd ) {
		SD3DAdapterInfo *adapterInfo = new SD3DAdapterInfo();
		if( adapterInfo == NULL ) return E_OUTOFMEMORY;

		mDirect3D->GetAdapterIdentifier( adOrd, 0, &adapterInfo->adapterID );
		adapterInfo->adapterOrdinal = adOrd;

		//
		// get list of all display modes on this adapter.  
		// also build a temporary list of all display adapter formats.

		TDwordVector adapterFormatList; // D3DFORMAT
		
		for( size_t ifmt = 0; ifmt < mAllowedFormats.size(); ++ifmt ) {
			D3DFORMAT format = (D3DFORMAT)mAllowedFormats[ifmt];
			int modeCount = mDirect3D->GetAdapterModeCount( adOrd, format );
			for( int mode = 0; mode < modeCount; ++mode ) {
				D3DDISPLAYMODE dm;
				mDirect3D->EnumAdapterModes( adOrd, format, mode, &dm );
				if( dm.Width < (UINT)mMinFullscreenWidth || 
					dm.Height < (UINT)mMinFullscreenHeight ||
					gGetColorChannelBits(dm.Format) < mMinColorChannelBits )
				{
					continue;
				}
				adapterInfo->displayModes.push_back( dm );
				if( std::find(adapterFormatList.begin(),adapterFormatList.end(),dm.Format) == adapterFormatList.end() )
					adapterFormatList.push_back( dm.Format );
			}
		}

		// sort displaymode list
		// TBD: no sort, causes some problems on some weird computers
		//std::sort( adapterInfo->displayModes.begin(), adapterInfo->displayModes.end() );
		
		// get info for each device on this adapter
		if( FAILED( hr = enumerateDevices( *adapterInfo, adapterFormatList ) ) ) {
			delete adapterInfo;
			return hr;
		}

		// if at least one device on this adapter is available and compatible
		// with the app, add adapterInfo to the list
		if( adapterInfo->deviceInfos.empty() ) {
			mIncompatAdapterInfos.push_back( adapterInfo );
		} else {
			mAdapterInfos.push_back( adapterInfo );
		}
	}

	return S_OK;
}




HRESULT CD3DEnumeration::enumerateDevices( SD3DAdapterInfo& adapterInfo, const TDwordVector& adapterFormats )
{
	const D3DDEVTYPE devTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };
	const int devTypeCount = sizeof(devTypes) / sizeof(devTypes[0]);
	
	HRESULT hr;
	
	for( int idt = 0; idt < devTypeCount; ++idt ) {
		D3DCAPS9 caps;
		if( FAILED( mDirect3D->GetDeviceCaps( adapterInfo.adapterOrdinal, devTypes[idt], &caps ) ) )
			continue;

		SD3DDeviceInfo* deviceInfo = new SD3DDeviceInfo();
		if( deviceInfo == NULL ) return E_OUTOFMEMORY;

		// check out device capabilities
		deviceInfo->caps.setData( *mDirect3D, adapterInfo.adapterOrdinal, devTypes[idt], caps );

		// if we can load config DB - load and verify requirements
		if( !loadAndVerifyConfigDB( adapterInfo, *deviceInfo ) ) {
			delete deviceInfo;
			continue;
		}

		// find&confirm suitable vertex processings (if any)
		// none can be found in a case that the device has no sufficient capabilities
		buildVertexProcessings( adapterInfo, *deviceInfo );
		if( deviceInfo->vertexProcessings.empty() ) {
			// TBD: add error message somewhere
			delete deviceInfo;
			continue;
		}
		
		// get info for each device combo on this device
		if( FAILED( hr = enumerateDeviceCombos( adapterInfo, *deviceInfo, adapterFormats ) ) ) {
			delete deviceInfo;
			return hr;
		}
		
		// if at least one combo is found, add the deviceInfo to the list
		if( deviceInfo->deviceCombos.empty() ) {
			// TBD: add error message somewhere
			delete deviceInfo;
			continue;
		}
		adapterInfo.deviceInfos.push_back( deviceInfo );
	}
	return S_OK;
}




HRESULT CD3DEnumeration::enumerateDeviceCombos( const SD3DAdapterInfo& adapterInfo, SD3DDeviceInfo& deviceInfo, const TDwordVector& adapterFormats )
{
	const D3DFORMAT bbufferFormats[] = {
		D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10,
		D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5
	};
	const int bbufferFormatCount = sizeof(bbufferFormats) / sizeof(bbufferFormats[0]);
	
	bool isWindowedArray[] = { false, true };
	
	//
	// see which adapter formats are supported by this device

	for( size_t iaf = 0; iaf < adapterFormats.size(); ++iaf ) {
		D3DFORMAT format = (D3DFORMAT)adapterFormats[iaf];
		for( int ibbf = 0; ibbf < bbufferFormatCount; ibbf++ ) {
			D3DFORMAT bbufferFormat = bbufferFormats[ibbf];
			if( gGetAlphaChannelBits(bbufferFormat) < mMinAlphaChannelBits )
				continue;
			for( int iiw = 0; iiw < 2; ++iiw ) {
				bool isWindowed = isWindowedArray[iiw];
				if( !isWindowed && mRequiresWindowed )
					continue;
				if( isWindowed && mRequiresFullscreen )
					continue;
				if( FAILED( mDirect3D->CheckDeviceType(
					deviceInfo.caps.getAdapterOrdinal(), deviceInfo.caps.getDeviceType(), 
					format, bbufferFormat, isWindowed ) ) )
				{
					continue;
				}

				// Here, we have an adapter/device/adapterformat/backbufferformat/iswindowed
				// combo that is supported by the system. We still need to
				// confirm that it's compatible with the app, and find one or
				// more suitable depth/stencil buffer format, multisample type,
				// vertex processing type, and present interval.
				SD3DDeviceCombo* devCombo = new SD3DDeviceCombo();
				if( devCombo == NULL ) return E_OUTOFMEMORY;

				devCombo->adapterOrdinal = deviceInfo.caps.getAdapterOrdinal();
				devCombo->deviceType = deviceInfo.caps.getDeviceType();
				devCombo->adapterFormat = format;
				devCombo->backBufferFormat = bbufferFormat;
				devCombo->isWindowed = isWindowed;
				if( mUsesDepthBuffer ) {
					buildDepthStencilFormats( *devCombo );
					if( devCombo->depthStencilFormats.empty() ) {
						delete devCombo;
						continue;
					}
				}
				buildMultiSampleTypes( *devCombo );
				if( devCombo->multiSampleTypes.empty() ) {
					delete devCombo;
					continue;
				}
				buildConflicts( *devCombo );

				buildVSyncs( deviceInfo, *devCombo );
				
				deviceInfo.deviceCombos.push_back( devCombo );
			}
		}
	}
	
	return S_OK;
}



void CD3DEnumeration::buildDepthStencilFormats( SD3DDeviceCombo& devCombo )
{
	const D3DFORMAT dsFormats[] = {
		D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D16, D3DFMT_D15S1, D3DFMT_D32,
	};
	const int dsFormatCount = sizeof(dsFormats) / sizeof(dsFormats[0]);
	
	for( int idsf = 0; idsf < dsFormatCount; ++idsf ) {
		D3DFORMAT format = dsFormats[idsf];
		if( gGetDepthBits(format) < mMinDepthBits )
			continue;
		if( gGetStencilBits(format) < mMinStencilBits )
			continue;
		if( SUCCEEDED( mDirect3D->CheckDeviceFormat(
			devCombo.adapterOrdinal, devCombo.deviceType, devCombo.adapterFormat, 
			D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, format ) ) )
		{
			if( SUCCEEDED( mDirect3D->CheckDepthStencilMatch(
				devCombo.adapterOrdinal, devCombo.deviceType, devCombo.adapterFormat, 
				devCombo.backBufferFormat, format ) ) )
			{
				devCombo.depthStencilFormats.push_back( format );
			}
		}
	}
}


void CD3DEnumeration::buildMultiSampleTypes( SD3DDeviceCombo& devCombo )
{
	const D3DMULTISAMPLE_TYPE msTypes[] = { 
		D3DMULTISAMPLE_NONE,
		D3DMULTISAMPLE_NONMASKABLE,
		D3DMULTISAMPLE_2_SAMPLES,
		D3DMULTISAMPLE_3_SAMPLES,
		D3DMULTISAMPLE_4_SAMPLES,
		D3DMULTISAMPLE_5_SAMPLES,
		D3DMULTISAMPLE_6_SAMPLES,
		D3DMULTISAMPLE_7_SAMPLES,
		D3DMULTISAMPLE_8_SAMPLES,
		D3DMULTISAMPLE_9_SAMPLES,
		D3DMULTISAMPLE_10_SAMPLES,
		D3DMULTISAMPLE_11_SAMPLES,
		D3DMULTISAMPLE_12_SAMPLES,
		D3DMULTISAMPLE_13_SAMPLES,
		D3DMULTISAMPLE_14_SAMPLES,
		D3DMULTISAMPLE_15_SAMPLES,
		D3DMULTISAMPLE_16_SAMPLES,
	};
	const size_t msTypeCount = sizeof(msTypes) / sizeof(msTypes[0]);
	
	for( size_t imst = 0; imst < msTypeCount; ++imst ) {
		D3DMULTISAMPLE_TYPE	msType = msTypes[imst];
		DWORD msQuality;
		if( SUCCEEDED( mDirect3D->CheckDeviceMultiSampleType(
			devCombo.adapterOrdinal, devCombo.deviceType, 
			devCombo.backBufferFormat, devCombo.isWindowed,
			msType, &msQuality ) ) )
		{
			devCombo.multiSampleTypes.push_back( msType );
			devCombo.multiSampleQualities.push_back( msQuality );
		}
	}
}


void CD3DEnumeration::buildConflicts( SD3DDeviceCombo& devCombo )
{
	
	for( size_t ids = 0; ids < devCombo.depthStencilFormats.size(); ++ids ) {
		D3DFORMAT format = (D3DFORMAT)devCombo.depthStencilFormats[ids];
		for( size_t ims = 0; ims < devCombo.multiSampleTypes.size(); ++ims ) {
			D3DMULTISAMPLE_TYPE msType = (D3DMULTISAMPLE_TYPE)devCombo.multiSampleTypes[ims];
			if( FAILED( mDirect3D->CheckDeviceMultiSampleType(
				devCombo.adapterOrdinal, devCombo.deviceType,
				format, devCombo.isWindowed, msType, NULL ) ) )
			{
				SD3DDeviceCombo::SDSMSConflict conflict;
				conflict.format = format;
				conflict.type = msType;
				devCombo.conflicts.push_back( conflict );
			}
		}
	}
};



void CD3DEnumeration::buildVertexProcessings( const SD3DAdapterInfo& adapterInfo, SD3DDeviceInfo& deviceInfo )
{
	assert( mConfirmDeviceCallback );
	DWORD devCaps = deviceInfo.caps.getCaps().DevCaps;
	if( devCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
		// try pure hw...
		if( devCaps & D3DDEVCAPS_PUREDEVICE ) {
			if( mConfirmDeviceCallback( deviceInfo.caps, CD3DDeviceCaps::VP_PURE_HW, adapterInfo.errors ) )
				deviceInfo.vertexProcessings.push_back( CD3DDeviceCaps::VP_PURE_HW );
		}
		// try non-pure hw...
		if( mConfirmDeviceCallback( deviceInfo.caps, CD3DDeviceCaps::VP_HW, adapterInfo.errors ) )
			deviceInfo.vertexProcessings.push_back( CD3DDeviceCaps::VP_HW );
		// try mixed...
		if( mUsesMixedVP ) {
			if( mConfirmDeviceCallback( deviceInfo.caps, CD3DDeviceCaps::VP_MIXED, adapterInfo.errors ) )
				deviceInfo.vertexProcessings.push_back( CD3DDeviceCaps::VP_MIXED );
		}
	}

	// try sw...
	if( mConfirmDeviceCallback( deviceInfo.caps, CD3DDeviceCaps::VP_SW, adapterInfo.errors ) )
		deviceInfo.vertexProcessings.push_back( CD3DDeviceCaps::VP_SW );
}

void CD3DEnumeration::buildVSyncs( const SD3DDeviceInfo& deviceInfo, SD3DDeviceCombo& devCombo )
{
	devCombo.hasNoVSync = ( deviceInfo.caps.getCaps().PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE ) != 0;
	devCombo.hasVSync = ( deviceInfo.caps.getCaps().PresentationIntervals & D3DPRESENT_INTERVAL_ONE ) != 0;
}

bool CD3DEnumeration::loadAndVerifyConfigDB( const SD3DAdapterInfo& adapterInfo, SD3DDeviceInfo& deviceInfo ) const
{
	// no cfg DB filename - do nothing
	if( !mConfigDBFileName )
		return true;

	// if error loading the DB - assume the device is ok, but assert in debug mode
	deviceInfo.configDB = IDXConfigDB::Create();
	if( !deviceInfo.configDB ) {
		assert( false );
		return true;
	}
	bool ok = deviceInfo.configDB->Load( mConfigDBFileName, deviceInfo.caps, 128 ); // TBD - system mem!
	if( !ok ) {
		assert( false );
		return true;
	}

	// see our device properties...
	bool result = true;
	char buf[300];
	const char* gfxVendor = deviceInfo.configDB->GetGfxVendorString();
	const char* gfxDevice = deviceInfo.configDB->GetGfxDeviceString();

	int nprops = deviceInfo.configDB->GetDevicePropertyCount();
	for( int i = 0; i < nprops; ++i ) {
		const char* name = deviceInfo.configDB->GetDeviceProperty( i );
		//const char* value = deviceInfo.configDB->GetDeviceValue( i );
		if( !stricmp( name, "UnsupportedCard" ) ) {
			sprintf( buf, "This video card is unsupported (%s / %s)", gfxVendor, gfxDevice );
			adapterInfo.errors.addError( buf );
			result = false;
		} else if( !stricmp( name, "InvalidDriver" ) ) {
			sprintf( buf, "Your video card driver is known to have really serious problems. Please update it." );
			adapterInfo.errors.addError( buf );
			result = false;
		}
	}

	return result;
}


