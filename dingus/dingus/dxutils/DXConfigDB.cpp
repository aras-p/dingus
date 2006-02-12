#include "stdafx.h"
#include "DXConfigDB.h"
#include <cstdio>

using namespace dingus;

// --------------------------------------------------------------------------

/*
static DWORD GetHexDigits( WCHAR* ptr ) {
	DWORD val = 0;
	while( ( *ptr >= L'0' && *ptr <= L'9' ) || ( *ptr >= L'a' && *ptr <= L'f' ) ) {
		val *= 16;
		if( *ptr >= L'0' && *ptr <= L'9' )
			val += *ptr - L'0';
		else
			val += *ptr - L'a' + 10;
		
		++ptr;
	};
	return val;
}


static void ConvertToLower( WCHAR *psz ) {
	while( *psz != L'\0' ) {
		if( *psz >= L'A' && *psz <= L'Z' )
			*psz += (L'a' - L'A');
		++psz;
	}
}
*/

class CDXConfibDBImpl;

const char kDBKwDisplayVendor[] 			= "displayvendor";
const char kDBKwApplyToAll[]				= "applytoall";
const char kDBKwRequirements[]				= "Requirements";
const char kDBKwPropertySet[]				= "propertyset";
const char kDBKwIf[]						= "if";
const char kDBKwEndif[] 					= "endif";
const char kDBKwRam[]						= "ram";
const char kDBKwVideoram[]					= "videoram";
const char kDBKwSubsysid[]					= "subsysid";
const char kDBKwRevision[]					= "revision";
const char kDBKwDriver[]					= "driver";
const char kDBKwGuid[]						= "guid";
const char kDBKwOs[]						= "os";
const char kDBKwWin95[] 					= "win95";
const char kDBKwWin98[] 					= "win98";
const char kDBKwWin98se[]					= "win98se";
const char kDBKwWinme[] 					= "winme";
const char kDBKwWin2k[] 					= "win2k";
const char kDBKwWinxp[] 					= "winxp";
const char kDBKwWin2003[]					= "win2003";
const char kDBKwBreak[] 					= "break";
const char kDBKwUnknown[]					= "unknown";
const char kDBKwCaps[]						= "Caps";
const char kDBKwCaps2[] 					= "Caps2";
const char kDBKwCaps3[] 					= "Caps3";
const char kDBKwPresentationIntervals[] 	= "PresentationIntervals";
const char kDBKwCursorCaps[]				= "CursorCaps";
const char kDBKwDevCaps[]					= "DevCaps";
const char kDBKwPrimitiveMiscCaps[] 		= "PrimitiveMiscCaps";
const char kDBKwRasterCaps[]				= "RasterCaps";
const char kDBKwZCmpCaps[]					= "ZCmpCaps";
const char kDBKwSrcBlendCaps[]				= "SrcBlendCaps";
const char kDBKwDestBlendCaps[] 			= "DestBlendCaps";
const char kDBKwAlphaCmpCaps[]				= "AlphaCmpCaps";
const char kDBKwShadeCaps[] 				= "ShadeCaps";
const char kDBKwTextureCaps[]				= "TextureCaps";
const char kDBKwTextureFilterCaps[] 		= "TextureFilterCaps";
const char kDBKwCubeTextureFilterCaps[] 	= "CubeTextureFilterCaps";
const char kDBKwVolumeTextureFilterCaps[]	= "VolumeTextureFilterCaps";
const char kDBKwTextureAddressCaps[]		= "TextureAddressCaps";
const char kDBKwVolumeTextureAddressCaps[]	= "VolumeTextureAddressCaps";
const char kDBKwLineCaps[]					= "LineCaps";
const char kDBKwMaxTextureWidth[]			= "MaxTextureWidth";
const char kDBKwMaxTextureHeight[]			= "MaxTextureHeight";
const char kDBKwMaxVolumeExtent[]			= "MaxVolumeExtent";
const char kDBKwMaxTextureRepeat[]			= "MaxTextureRepeat";
const char kDBKwMaxTextureAspectRatio[] 	= "MaxTextureAspectRatio";
const char kDBKwMaxAnisotropy[] 			= "MaxAnisotropy";
const char kDBKwStencilCaps[]				= "StencilCaps";
const char kDBKwFVFCaps[]					= "FVFCaps";
const char kDBKwTextureOpCaps[] 			= "TextureOpCaps";
const char kDBKwMaxTextureBlendStages[] 	= "MaxTextureBlendStages";
const char kDBKwMaxSimultaneousTextures[]	= "MaxSimultaneousTextures";
const char kDBKwVertexProcessingCaps[]		= "VertexProcessingCaps";
const char kDBKwMaxActiveLights[]			= "MaxActiveLights";
const char kDBKwMaxUserClipPlanes[] 		= "MaxUserClipPlanes";
const char kDBKwMaxVertexBlendMatrices[]	= "MaxVertexBlendMatrices";
const char kDBKwMaxVertexBlendMatrixIndex[] = "MaxVertexBlendMatrixIndex";
const char kDBKwMaxPrimitiveCount[] 		= "MaxPrimitiveCount";
const char kDBKwMaxVertexIndex[]			= "MaxVertexIndex";
const char kDBKwMaxStreams[]				= "MaxStreams";
const char kDBKwMaxStreamStride[]			= "MaxStreamStride";
const char kDBKwVertexShaderVersion[]		= "VertexShaderVersion";
const char kDBKwMaxVertexShaderConst[]		= "MaxVertexShaderConst";
const char kDBKwPixelShaderVersion[]		= "PixelShaderVersion";
const char kDBKwCPUSpeed[]					= "cpuspeed";
const char kDBKwOverallGraphicDetail[]		= "OverallGraphicDetail";
const char kDBKwMaxOverallGraphicDetail[]	= "MaxOverallGraphicDetail";


// --------------------------------------------------------------------------
//  CPropertySet
// --------------------------------------------------------------------------

class CPropertySet {
public:
	CPropertySet( const CDXConfibDBImpl& owner ) : mOwner( owner ) { }
	
	void Set(const char* Property, const char * Value);
	bool Get(const char* Property, char* Value, int cbValue);
	const std::vector<StringPair>& GetProperties() const { return mProps; }

private:
	std::vector<StringPair> mProps;
	const CDXConfibDBImpl& mOwner;
};

void CPropertySet::Set(const char* Property, const char* Value)
{
	std::vector<StringPair>::iterator it, end;
	it = mProps.begin();
	end = mProps.end();
	while (it != end)
	{
		StringPair& pr = *it;
		if (0 == _stricmp(pr.first.c_str(), Property))
		{
			pr.second = Value;
			return;
		}
		++it;
	}
	mProps.push_back(StringPair(std::string(Property), std::string(Value)));
}

bool CPropertySet::Get(const char* Property, char* Value, int cbValue)
{
	std::vector<StringPair>::iterator it, end;
	it = mProps.begin();
	end = mProps.end();
	while (it != end)
	{
		StringPair& pr = *it;
		if (0 == _stricmp(pr.first.c_str(), Property))
		{
			strncpy(Value, pr.second.c_str(), cbValue);
			return true;
		}
		++it;
	}
	return false;
}

// --------------------------------------------------------------------------
// CDXConfibDBImpl
// --------------------------------------------------------------------------

class CDXConfibDBImpl : public IDXConfigDB {
public:
	CDXConfibDBImpl();
	~CDXConfibDBImpl();
	
	//
	// IDXConfigDB

	bool Load( const WCHAR* FileName, const CD3DDeviceCaps& caps, DWORD SystemMemory );
	void Release() {delete this;};
	
	unsigned int GetDevicePropertyCount() const { return (int)m_pDevice->GetProperties().size(); }
	const char* GetDeviceProperty(unsigned int i) const {return m_pDevice->GetProperties().at(i).first.c_str(); }
	const char* GetDeviceValue(unsigned int i) const { return m_pDevice->GetProperties().at(i).second.c_str(); }
	
	unsigned int GetRequirementsPropertyCount() const { return (int)m_pRequirements->GetProperties().size(); }
	const char* GetRequirementsProperty(unsigned int i) const { return m_pRequirements->GetProperties().at(i).first.c_str(); }
	const char* GetRequirementsValue(unsigned int i) const { return m_pRequirements->GetProperties().at(i).second.c_str(); }
	
	const std::vector<StringPair>& GetAggregateProperties() const { return m_pDevice->GetProperties(); }
	const std::vector<StringPair>* GetNamedProperties(const char* ) const;
	const char* GetGfxDeviceString() const { return m_DeviceString.c_str(); }
	const char* GetGfxVendorString() const { return m_VendorString.c_str(); }
	
	bool IsError() const { return m_fError; }
	const char* GetErrorString() const { return m_ErrorString.c_str(); }
	
private:
	
	//
	//
	// Variables use to read in file
	//
	//
	char* m_pchFile;							// Pointer to start of file
	char* m_pchCurrent; 						// Current file pointer
	char* m_pchEndOfFile;						// Pointer to the end of file
	char* m_pchCurrentLine; 					// Pointer to start of this line
	DWORD m_LineNumber; 						// Line Number
	
	CPropertySet* m_pDevice;
	CPropertySet* m_pRequirements;
	
	bool m_fError;								// Errors adding strings
	std::string m_ErrorString;
	
	std::string m_DeviceString;
	std::string m_VendorString;
	

	CD3DDeviceCaps mCaps;		// Device caps passed in

	DWORD	m_SystemMemory;		// System memory passed in
	
	std::map<std::string, CPropertySet*> m_mapNameToPropertySet;
	std::map<std::string, CPropertySet*> m_mapOGDToPropertySet; //OGD==OverallGraphicDetail
	
	void SkipToNextLine();
	void SkipSpace();
	void SyntaxError( char* ErrorText );
	char* AddFlag(CPropertySet* pPropertySet);
	DWORD GetDigit();
	DWORD GetNumber();
	char* GetString();
	DWORD Get4Digits();
	DWORD sGet4Digits();
	char* GetCondition();
	bool DoDisplayVendorAndDevice();
	bool DoPropertySet(CPropertySet* pPropertySet);
	bool DoPropertySets();
	bool DoRequirements();
	void ApplyPropertySet(CPropertySet* pSetDst, CPropertySet* pSetSrc);
	bool DoPreApplyToAll();
	bool DoPostApplyToAll();
	bool NextStringIs(const char* psz);
};

const std::vector<StringPair>* CDXConfibDBImpl::GetNamedProperties(const char* pszName) const
{
	std::string strName(pszName);
	std::map<std::string, CPropertySet*>::const_iterator it;
	it = m_mapNameToPropertySet.find(strName);
	if (it == m_mapNameToPropertySet.end())
	{
		return NULL;
	}
	CPropertySet* pSet = (*it).second;
	return &pSet->GetProperties();
}


//
// Skip to start of next line
//
void CDXConfibDBImpl::SkipToNextLine()
{
	do
	{
		m_pchCurrent++;
	} while( *(m_pchCurrent-1)!=13 && m_pchCurrent<m_pchEndOfFile );
	
	if( m_pchCurrent<m_pchEndOfFile && *m_pchCurrent==10 )
	{
		m_pchCurrent++;
	}
	
	m_pchCurrentLine=m_pchCurrent;
	m_LineNumber++;
}

//
// Skip over blank spaces
//
void CDXConfibDBImpl::SkipSpace()
{
	while( *m_pchCurrent==' ' || *m_pchCurrent==9 )
		m_pchCurrent++;
}

//
// Generic syntax error
//
void CDXConfibDBImpl::SyntaxError( char* ErrorText )
{
	if( !m_fError )
	{
		char tempBuffer[40];
		char* dest=tempBuffer;
		char* source=m_pchCurrentLine;
		while( *source!=13 && dest!=&tempBuffer[36] )
		{
			*dest++=*source++;			// Copy current line into error buffer
		}
		if( dest==&tempBuffer[36] )
		{
			*dest++='.';
			*dest++='.';
			*dest++='.';
		}
		*dest=0;
		
		const size_t k_cchBuffer = 256;
		char Buffer[k_cchBuffer];
		_snprintf( Buffer, k_cchBuffer, "%s on line %d - '%s'", ErrorText, m_LineNumber, tempBuffer );
		Buffer[k_cchBuffer-1] = 0;
		m_ErrorString = Buffer;
		m_fError=1;
	}
}

//
// Return a hex digit -1=m_fError  and move pointer on
//
DWORD CDXConfibDBImpl::GetDigit()
{
	DWORD result = (DWORD)-1;
	
	if( *m_pchCurrent>='0' && *m_pchCurrent<='9' )
	{
		result=(DWORD)(*m_pchCurrent++)-'0';
	}
	else
	{
		if( *m_pchCurrent>='a' && *m_pchCurrent<='f' )
		{
			result=(DWORD)(*m_pchCurrent++)-'a'+10;
		}
		else
		{
			if( *m_pchCurrent>='A' && *m_pchCurrent<='F' )
			{
				result=(DWORD)(*m_pchCurrent++)-'A'+10;
			}
		}
	}
	
	return result;
}

//
// A number is expected, get it - return -1 for error
//
DWORD CDXConfibDBImpl::GetNumber()
{
	DWORD result;
	
	SkipSpace();
	
	if( *(WORD*)m_pchCurrent!='x0' )
	{
		if( *m_pchCurrent>='0' && *m_pchCurrent<='9' )
		{
			//
			// Decimal number
			//
			result=GetDigit();
			
			if( result==-1 )
			{
				SyntaxError( "Number expected" );
				return (DWORD)-1;
			}
			
			while( *m_pchCurrent>='0' && *m_pchCurrent<='9' )
			{
				DWORD tmp = GetDigit();
				if( tmp == (DWORD)-1 )
					break;
				
				if( result>=16602069666338596456 )
				{
					SyntaxError( "Number too large" );		
					return (DWORD)-1;
				}
				
				result=result*10+tmp;
			}
			
			SkipSpace();
			
			return result;
			
		}
		else
		{
			SyntaxError( "Number expected" );		
			return (DWORD)-1;
		}
	}
	//
	// Hex number?
	//
	m_pchCurrent+=2;
	
	DWORD tmp=GetDigit();
	
	if( tmp == (DWORD)-1 )
	{
		SyntaxError( "Number expected" );
		return (DWORD)-1;
	}
	
	result = 0;
	
	DWORD hexcount = 0;
	do
	{
		if( hexcount>=8 )
		{
			SyntaxError( "Number too large" );		
			return (DWORD)-1;
		}
		
		result=result*16+tmp;
		++hexcount;
		tmp = GetDigit();
	} while (tmp != -1);
	
	SkipSpace();
	
	return result;
}

//
// A string is expected, get it - return 0 for error
//
char* CDXConfibDBImpl::GetString()
{
	static char Buffer[256];
	
	SkipSpace();
	
	if( *m_pchCurrent++!='"' )
	{
		SyntaxError( "Expecting """ );		
		return 0;
	}	
	
	char* dest=Buffer;
	
	while( *m_pchCurrent!='"' )
	{
		*dest++=*m_pchCurrent++;
		
		if( dest>&Buffer[255] )
		{
			SyntaxError( "String too long" );		
			return 0;
		}	
	}
	
	*dest=0;
	m_pchCurrent++;
	
	SkipSpace();
	
	return Buffer;
}

//
// Pointing at a flag = value line, add to the flags being returned
//
char* CDXConfibDBImpl::AddFlag(CPropertySet* pPropertySet)
{
	char Flag[256];
	char* dest=Flag;
	
	while( *m_pchCurrent!=' ' && *m_pchCurrent!='=' && *m_pchCurrent!=13 )
	{
		char chr=*m_pchCurrent++;
		
		if( chr>='A' && chr<='Z' )
			chr+='a'-'A';					// Convert to lower case
		
		*dest++=chr;
		
		if( dest==&Flag[254] )
		{
			return "Flag too long";
		}						
	}
	*dest=0;
	
	SkipSpace();
	
	char Value[256];
	dest=Value;
	
	if( *m_pchCurrent!=13 )
	{
		if( *m_pchCurrent!='=' )
		{
			return "flag = xxx expected";
		}
		
		m_pchCurrent++;
		SkipSpace();
		
		if( *m_pchCurrent=='"' )
		{
			do
			{
				char chr=*m_pchCurrent++;
				
				*dest++=chr;
				
				if( dest==&Value[254] )
				{
					return "Flag too long";
				}						
				
			} while( *m_pchCurrent!='"' && *m_pchCurrent!=13 );
			
			if( *m_pchCurrent!='"' )
			{
				return "Missing Quote";
			}
			
			*dest++='"';
			
			if( dest==&Value[254] )
			{
				return "Flag too long";
			}						
		}
		else
		{
			while( *m_pchCurrent!=' ' && *m_pchCurrent!=13 )
			{
				char chr=*m_pchCurrent++;
				
				if( chr>='A' && chr<='Z' )
					chr+='a'-'A';					// Convert to lower case
				
				*dest++=chr;
				
				if( dest==&Value[254] )
				{
					return "Flag too long";
				}						
			}
		}
	}
	*dest=0;
	
	pPropertySet->Set(Flag, Value);
	
	//	Check for OverallGraphicDetail. This is a special keyword. It is
	//	used to add this property set to another map.
	if (_stricmp(kDBKwOverallGraphicDetail, Flag) == 0)
		m_mapOGDToPropertySet[Value] = pPropertySet;
	
	return 0;
}

//
// Return true if the character is not a alphanumeric
//
bool NotAscii( char Chr )
{
	if( Chr=='>' || Chr=='<' || Chr=='!' || Chr=='=' || Chr==' ' || Chr==13 || Chr==9 )
		return true;
	
	return false;
}

//
// Returns the next 4 hex digits, -1 if any errors
//
DWORD CDXConfibDBImpl::Get4Digits()
{
	DWORD result;
	
	DWORD tmp=GetDigit();
	
	if( tmp == (DWORD)-1 )
		return (DWORD)-1;
	
	result=tmp<<12;
	
	tmp=GetDigit();
	
	if( tmp == (DWORD)-1 )
		return (DWORD)-1;
	
	result|=tmp<<8;
	
	tmp=GetDigit();
	
	if( tmp == (DWORD)-1 )
		return (DWORD)-1;
	
	result|=tmp<<4;
	
	tmp=GetDigit();
	
	if( tmp == (DWORD)-1 )
		return (DWORD)-1;
	
	result|=tmp;
	
	return result;
}

//
// Returns the next 4 hex digits, -1 if any errors - SWAPPED version (for guids)
//
DWORD CDXConfibDBImpl::sGet4Digits()
{
	DWORD tmp = Get4Digits();
	
	if( tmp == (DWORD)-1 )
		return (DWORD)-1;
	
	return ((tmp&0xff00)>>8) + ((tmp&0xff)<<8);
}

bool CDXConfibDBImpl::NextStringIs(const char* psz)
{
	int cch = (int)strlen(psz);
	return (0==_strnicmp( m_pchCurrent, psz, cch ) && NotAscii(m_pchCurrent[cch]));
}

//
// Evaluate an IF condition
//
// 1=True, 0=False, n=m_fError message
//
//
char* CDXConfibDBImpl::GetCondition()
{
	DWORD Source=0;
	DWORD Type=0;		// 0=Value, 1=GUID, 2=Driver, 3=os
	DWORD Compare=0;	// 0 ==,	1 !=,	2 >,   3 <,   4 >=,    5 <=
	
	SkipSpace();
	
	if( NextStringIs(kDBKwCPUSpeed))
	{
		Source=mCaps.getCpuMhz();
		m_pchCurrent+=strlen(kDBKwCPUSpeed);
	}
	else if( NextStringIs(kDBKwRam))
	{
		Source=m_SystemMemory;
		m_pchCurrent+=strlen(kDBKwRam);
	}
	else if ( NextStringIs(kDBKwCaps))
	{
		Source=mCaps.getCaps().Caps;
		m_pchCurrent+=strlen(kDBKwCaps);
	}
	else if ( NextStringIs(kDBKwCaps2))
	{
		Source=mCaps.getCaps().Caps2;
		m_pchCurrent+=strlen(kDBKwCaps2);
	}
	else if ( NextStringIs(kDBKwCaps3))
	{
		Source=mCaps.getCaps().Caps3;
		m_pchCurrent+=strlen(kDBKwCaps3);
	}
	else if ( NextStringIs(kDBKwPresentationIntervals))
	{
		Source=mCaps.getCaps().PresentationIntervals;
		m_pchCurrent+=strlen(kDBKwPresentationIntervals);
	}
	else if ( NextStringIs(kDBKwCursorCaps))
	{
		Source=mCaps.getCaps().CursorCaps;
		m_pchCurrent+=strlen(kDBKwCursorCaps);
	}
	else if ( NextStringIs(kDBKwDevCaps))
	{
		Source=mCaps.getCaps().DevCaps;
		m_pchCurrent+=strlen(kDBKwDevCaps);
	}
	else if ( NextStringIs(kDBKwPrimitiveMiscCaps))
	{
		Source=mCaps.getCaps().PrimitiveMiscCaps;
		m_pchCurrent+=strlen(kDBKwPrimitiveMiscCaps);
	}
	else if ( NextStringIs(kDBKwRasterCaps))
	{
		Source=mCaps.getCaps().RasterCaps;
		m_pchCurrent+=strlen(kDBKwRasterCaps);
	}
	else if ( NextStringIs(kDBKwZCmpCaps))
	{
		Source=mCaps.getCaps().ZCmpCaps;
		m_pchCurrent+=strlen(kDBKwZCmpCaps);
	}
	else if ( NextStringIs(kDBKwSrcBlendCaps))
	{
		Source=mCaps.getCaps().SrcBlendCaps;
		m_pchCurrent+=strlen(kDBKwSrcBlendCaps);
	}
	else if ( NextStringIs(kDBKwDestBlendCaps))
	{
		Source=mCaps.getCaps().DestBlendCaps;
		m_pchCurrent+=strlen(kDBKwDestBlendCaps);
	}
	else if ( NextStringIs(kDBKwAlphaCmpCaps))
	{
		Source=mCaps.getCaps().AlphaCmpCaps;
		m_pchCurrent+=strlen(kDBKwAlphaCmpCaps);
	}
	else if ( NextStringIs(kDBKwShadeCaps))
	{
		Source=mCaps.getCaps().ShadeCaps;
		m_pchCurrent+=strlen(kDBKwShadeCaps);
	}
	else if ( NextStringIs(kDBKwTextureCaps))
	{
		Source=mCaps.getCaps().TextureCaps;
		m_pchCurrent+=strlen(kDBKwTextureCaps);
	}
	else if ( NextStringIs(kDBKwTextureFilterCaps))
	{
		Source=mCaps.getCaps().TextureFilterCaps;
		m_pchCurrent+=strlen(kDBKwTextureFilterCaps);
	}
	else if ( NextStringIs(kDBKwCubeTextureFilterCaps))
	{
		Source=mCaps.getCaps().CubeTextureFilterCaps;
		m_pchCurrent+=strlen(kDBKwCubeTextureFilterCaps);
	}
	else if ( NextStringIs(kDBKwVolumeTextureFilterCaps))
	{
		Source=mCaps.getCaps().VolumeTextureFilterCaps;
		m_pchCurrent+=strlen(kDBKwVolumeTextureFilterCaps);
	}
	else if ( NextStringIs(kDBKwTextureAddressCaps))
	{
		Source=mCaps.getCaps().TextureAddressCaps;
		m_pchCurrent+=strlen(kDBKwTextureAddressCaps);
	}
	else if ( NextStringIs(kDBKwVolumeTextureAddressCaps))
	{
		Source=mCaps.getCaps().VolumeTextureAddressCaps;
		m_pchCurrent+=strlen(kDBKwVolumeTextureAddressCaps);
	}
	else if ( NextStringIs(kDBKwLineCaps))
	{
		Source=mCaps.getCaps().LineCaps;
		m_pchCurrent+=strlen(kDBKwLineCaps);
	}
	else if ( NextStringIs(kDBKwMaxTextureWidth))
	{
		Source=mCaps.getCaps().MaxTextureWidth;
		m_pchCurrent+=strlen(kDBKwMaxTextureWidth);
	}
	else if ( NextStringIs(kDBKwMaxVolumeExtent))
	{
		Source=mCaps.getCaps().MaxVolumeExtent;
		m_pchCurrent+=strlen(kDBKwMaxVolumeExtent);
	}
	else if ( NextStringIs(kDBKwMaxTextureRepeat))
	{
		Source=mCaps.getCaps().MaxTextureRepeat;
		m_pchCurrent+=strlen(kDBKwMaxTextureRepeat);
	}
	else if ( NextStringIs(kDBKwMaxTextureAspectRatio))
	{
		Source=mCaps.getCaps().MaxTextureAspectRatio;
		m_pchCurrent+=strlen(kDBKwMaxTextureAspectRatio);
	}
	else if ( NextStringIs(kDBKwMaxAnisotropy))
	{
		Source=mCaps.getCaps().MaxAnisotropy;
		m_pchCurrent+=strlen(kDBKwMaxAnisotropy);
	}
	else if ( NextStringIs(kDBKwStencilCaps))
	{
		Source=mCaps.getCaps().StencilCaps;
		m_pchCurrent+=strlen(kDBKwStencilCaps);
	}
	else if ( NextStringIs(kDBKwFVFCaps))
	{
		Source=mCaps.getCaps().FVFCaps;
		m_pchCurrent+=strlen(kDBKwFVFCaps);
	}
	else if ( NextStringIs(kDBKwTextureOpCaps))
	{
		Source=mCaps.getCaps().TextureOpCaps;
		m_pchCurrent+=strlen(kDBKwTextureOpCaps);
	}
	else if ( NextStringIs(kDBKwMaxTextureBlendStages))
	{
		Source=mCaps.getCaps().MaxTextureBlendStages;
		m_pchCurrent+=strlen(kDBKwMaxTextureBlendStages);
	}
	else if ( NextStringIs(kDBKwMaxSimultaneousTextures))
	{
		Source=mCaps.getCaps().MaxSimultaneousTextures;
		m_pchCurrent+=strlen(kDBKwMaxSimultaneousTextures);
	}
	else if ( NextStringIs(kDBKwVertexProcessingCaps))
	{
		Source=mCaps.getCaps().VertexProcessingCaps;
		m_pchCurrent+=strlen(kDBKwVertexProcessingCaps);
	}
	else if ( NextStringIs(kDBKwMaxActiveLights))
	{
		Source=mCaps.getCaps().MaxActiveLights;
		m_pchCurrent+=strlen(kDBKwMaxActiveLights);
	}
	else if ( NextStringIs(kDBKwMaxUserClipPlanes))
	{
		Source=mCaps.getCaps().MaxUserClipPlanes;
		m_pchCurrent+=strlen(kDBKwMaxUserClipPlanes);
	}
	else if ( NextStringIs(kDBKwMaxVertexBlendMatrices))
	{
		Source=mCaps.getCaps().MaxVertexBlendMatrices;
		m_pchCurrent+=strlen(kDBKwMaxVertexBlendMatrices);
	}
	else if ( NextStringIs(kDBKwMaxVertexBlendMatrixIndex))
	{
		Source=mCaps.getCaps().MaxVertexBlendMatrixIndex;
		m_pchCurrent+=strlen(kDBKwMaxVertexBlendMatrixIndex);
	}
	else if ( NextStringIs(kDBKwMaxPrimitiveCount))
	{
		Source=mCaps.getCaps().MaxPrimitiveCount;
		m_pchCurrent+=strlen(kDBKwMaxPrimitiveCount);
	}
	else if ( NextStringIs(kDBKwMaxVertexIndex))
	{
		Source=mCaps.getCaps().MaxVertexIndex;
		m_pchCurrent+=strlen(kDBKwMaxVertexIndex);
	}
	else if ( NextStringIs(kDBKwMaxStreams))
	{
		Source=mCaps.getCaps().MaxStreams;
		m_pchCurrent+=strlen(kDBKwMaxStreams);
	}
	else if ( NextStringIs(kDBKwMaxStreamStride))
	{
		Source=mCaps.getCaps().MaxStreamStride;
		m_pchCurrent+=strlen(kDBKwMaxStreamStride);
	}
	else if ( NextStringIs(kDBKwVertexShaderVersion))
	{
		Source=mCaps.getCaps().VertexShaderVersion;
		m_pchCurrent+=strlen(kDBKwVertexShaderVersion);
	}
	else if ( NextStringIs(kDBKwMaxVertexShaderConst))
	{
		Source=mCaps.getCaps().MaxVertexShaderConst;
		m_pchCurrent+=strlen(kDBKwMaxVertexShaderConst);
	}
	else if ( NextStringIs(kDBKwPixelShaderVersion))
	{
		Source=mCaps.getCaps().PixelShaderVersion;
		m_pchCurrent+=strlen(kDBKwPixelShaderVersion);
	}
	else if (NextStringIs(kDBKwVideoram))
	{
		Source=mCaps.getVideoMemoryMB();
		m_pchCurrent+=8;
	}
	else if (NextStringIs(kDBKwSubsysid))
	{
		Source=mCaps.getAdapterID().SubSysId;
		m_pchCurrent+=8;
	}
	else if (NextStringIs(kDBKwRevision))
	{
		Source=mCaps.getAdapterID().Revision;
		m_pchCurrent+=8;
	}
	else if (NextStringIs(kDBKwGuid))
	{
		Type=1;
		m_pchCurrent+=4;
	}
	else if (NextStringIs(kDBKwDriver))
	{
		Type=2;
		m_pchCurrent+=6;
	}
	else if (NextStringIs(kDBKwOs))
	{
		OSVERSIONINFO osinfo;
		osinfo.dwOSVersionInfoSize=sizeof(osinfo);
		GetVersionEx( &osinfo );
		
		if( osinfo.dwPlatformId==VER_PLATFORM_WIN32_NT )
		{
			if( osinfo.dwMajorVersion==5 && osinfo.dwMinorVersion==2 )
				Source=6;  // Win2003
			else
			{
				Source=5;
				if( osinfo.dwMajorVersion==5 && osinfo.dwBuildNumber<2600 )
					Source=4;
			}
		}
		else
		{
			Source=3;
			if( (osinfo.dwBuildNumber&0xffff)<=2222 )
				Source=2;
			if( (osinfo.dwBuildNumber&0xffff)<=1998 )
				Source=1;
			if( (osinfo.dwBuildNumber&0xffff)<=950 )
				Source=0;
		}
		Type=3;
		m_pchCurrent+=2;
	}
	else
	{
		return "Unknown value";
	}
	
	SkipSpace();
	
	if( *(WORD*)m_pchCurrent=='==' )
	{
		m_pchCurrent+=2;
		Compare=0;
	}
	else
	{
		if( *(WORD*)m_pchCurrent=='=!' )
		{
			m_pchCurrent+=2;
			Compare=1;
		}
		else
		{
			if( *(WORD*)m_pchCurrent=='><' )
			{
				m_pchCurrent+=2;
				Compare=1;
			}
			else
			{
				if( *(WORD*)m_pchCurrent=='>=' )
				{
					m_pchCurrent+=2;
					Compare=4;
				}
				else
				{
					if( *(WORD*)m_pchCurrent=='<=' )
					{
						m_pchCurrent+=2;
						Compare=5;
					}
					else
					{
						if( *(WORD*)m_pchCurrent=='=<' )
						{
							m_pchCurrent+=2;
							Compare=5;
						}
						else
						{
							if( *(WORD*)m_pchCurrent=='=>' )
							{
								m_pchCurrent+=2;
								Compare=4;
							}
							else
							{
								if( *m_pchCurrent=='=' )
								{
									m_pchCurrent+=1;
									Compare=0;
								}
								else
								{
									if( *m_pchCurrent=='>' )
									{
										m_pchCurrent+=1;
										Compare=2;
									}
									else
									{
										if( *m_pchCurrent=='<' )
										{
											m_pchCurrent+=1;
											Compare=3;
										}
										else
										{
											if ( *m_pchCurrent=='&' )
											{
												m_pchCurrent+=1;
												Compare=6;
											}
											else
											{
												return "Unknown operator";
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	SkipSpace();
	
	if( Type==1 )								// GUID  D7B71F83-6340-11CF-4C73-0100A7C2C935
	{
		if( Compare>1 )
		{
			return "Only == or != allowed";
		}
		
		DWORD tempGUID[4];
		
		DWORD res=Get4Digits();
		
		if( res==-1 )
			return "Invalid GUID";
		
		DWORD res2=Get4Digits();
		
		if( res2==-1 )
			return "Invalid GUID";
		
		tempGUID[0] = (res<<16)+res2;
		
		if( *m_pchCurrent++!='-' )
		{
			return "Invalid GUID";
		}
		
		res=Get4Digits();
		
		if( res==-1 )
			return "Invalid GUID";
		
		if( *m_pchCurrent++!='-' )
		{
			return "Invalid GUID";
		}
		
		res2=Get4Digits();
		
		if( res2==-1 )
			return "Invalid GUID";
		
		tempGUID[1] = (res2<<16)+res;
		
		if( *m_pchCurrent++!='-' )
		{
			return "Invalid GUID";
		}
		
		res=sGet4Digits();
		
		if( res==-1 )
			return "Invalid GUID";
		
		if( *m_pchCurrent++!='-' )
		{
			return "Invalid GUID";
		}
		
		res2=sGet4Digits();
		
		if( res2==-1 )
			return "Invalid GUID";
		
		tempGUID[2] = (res2<<16)+res;
		
		res=sGet4Digits();
		
		if( res==-1 )
			return "Invalid GUID";
		
		res2=sGet4Digits();
		
		if( res2==-1 )
			return "Invalid GUID";
		
		tempGUID[3] = (res2<<16)+res;
		
		if( Compare==0 )
		{
			if( 0==memcmp( tempGUID, &mCaps.getAdapterID().DeviceIdentifier, 16 ) )
				return (char*)1;
			else
				return (char*)0;
		}
		
		if( 0!=memcmp( tempGUID, &mCaps.getAdapterID().DeviceIdentifier, 16 ) )
			return (char*)1;
		else
			return (char*)0;
		
	}
	
	if( Type==2 )								// Driver 4.1.25.1111
	{
		LARGE_INTEGER Driver;
		
		DWORD result=GetNumber();
		
		if( result==-1 || *m_pchCurrent++!='.' )
			return "Invalid driver number";
		
		DWORD result1=GetNumber();
		
		Driver.HighPart=(result<<16)+result1;
		
		if( result==-1 || *m_pchCurrent++!='.' )
			return "Invalid driver number";
		
		result=GetNumber();
		
		if( result==-1 || *m_pchCurrent++!='.' )
			return "Invalid driver number";
		
		result1=GetNumber();
		
		Driver.LowPart=(result<<16)+result1;
		
		if( result==-1 )
			return "Invalid driver number";
		
		__int64 rhs = *(__int64*)&Driver;
		__int64 lhs = *(__int64*)&mCaps.getAdapterID().DriverVersion;
		
		switch( Compare )
		{
		case 0: 								// ==
			if( lhs==rhs )
				return (char*)1;
			else
				return (char*)0;
		case 1: 								// !=
			if( lhs!=rhs )
				return (char*)1;
			else
				return (char*)0;
		case 2: 								// >
			if( lhs>rhs )
				return (char*)1;
			else
				return (char*)0;
		case 3: 								// <
			if( lhs<rhs )
				return (char*)1;
			else
				return (char*)0;
		case 4: 								// >=
			if( lhs>=rhs )
				return (char*)1;
			else
				return (char*)0;
		case 5: 								// <=
			if( lhs<=rhs )
				return (char*)1;
			else
				return (char*)0;
		}
		
		return "Invalid";
	}
	
	//
	// Get Value (or OS)

	DWORD Value;
	
	if( Type==3 ) {
		if (NextStringIs(kDBKwWin95)) {
			Value=0;
			m_pchCurrent+=5;			
		} else if (NextStringIs(kDBKwWin98)) {
			Value=1;
			m_pchCurrent+=5;			
		} else if (NextStringIs(kDBKwWin98se)) {
			Value=2;
			m_pchCurrent+=7;
		} else if (NextStringIs(kDBKwWinme)) {
			Value=3;
			m_pchCurrent+=5;
		} else if (NextStringIs(kDBKwWin2k)) {
			Value=4;
			m_pchCurrent+=5;
		} else if (NextStringIs(kDBKwWinxp)) {
			Value=5;
			m_pchCurrent+=5;			
		} else if (NextStringIs(kDBKwWin2003)) {
			Value=6;
			m_pchCurrent+=7;
		} else {
			return "Unknown OS";
		}

	} else {
		Value=GetNumber();
		if( Value==-1 )
			return "Number expected";
	}
	
	switch( Compare ) {
	case 0: 								// ==
		if( Source==Value )
			return (char*)1;
		else
			return (char*)0;
	case 1: 								// !=
		if( Source!=Value )
			return (char*)1;
		else
			return (char*)0;
	case 2: 								// >
		if( Source>Value )
			return (char*)1;
		else
			return (char*)0;
	case 3: 								// <
		if( Source<Value )
			return (char*)1;
		else
			return (char*)0;
	case 4: 								// >=
		if( Source>=Value )
			return (char*)1;
		else
			return (char*)0;
	case 5: 								// <=
		if( Source<=Value )
			return (char*)1;
		else
			return (char*)0;
		
	case 6: 								// &
		if ( Source & Value )
			return (char*)1;
		else
			return (char*)0;
	}
	
	return "Invalid";
}
	
bool CDXConfibDBImpl::Load( const WCHAR* FileName, const CD3DDeviceCaps& caps, DWORD SystemMemory )
{
	if( m_pDevice == NULL )
		m_pDevice = new CPropertySet( *this );

	if( m_pRequirements == NULL )
		m_pRequirements = new CPropertySet( *this );
	
	//
	// remember passed params

	mCaps = caps;
	m_SystemMemory = SystemMemory;
	m_fError = false;

	//
	// If driver version is 0.0.0.0 try and read from driver .dll
	//
	if( (mCaps.getAdapterID().DriverVersion.HighPart|mCaps.getAdapterID().DriverVersion.LowPart) == 0 )
	{
		VS_FIXEDFILEINFO ffi;
		
		DWORD dwHandle;
		DWORD cchver = GetFileVersionInfoSizeA( mCaps.getAdapterID().Driver, &dwHandle );
		if( cchver!=0 ) 
		{
			char* pver = new char[cchver];
			
			BOOL bret = GetFileVersionInfoA( mCaps.getAdapterID().Driver, dwHandle, cchver, pver );
			
			if( bret ) 
			{
				UINT uLen;
				void *pbuf;
				
				bret = VerQueryValueA( pver, "\\", &pbuf, &uLen );
				
				if( bret ) 
				{
					memcpy( &ffi, pbuf, sizeof(VS_FIXEDFILEINFO) );
					
					mCaps.getAdapterID().DriverVersion.HighPart=ffi.dwFileVersionMS;
					mCaps.getAdapterID().DriverVersion.LowPart=ffi.dwFileVersionLS;
				}
			}
			
			delete [] pver;
		}
	}
	//
	// Try and find video card file
	//
	HANDLE hFile = CreateFileW( FileName, GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, 0, NULL );
	
	if( hFile == INVALID_HANDLE_VALUE )
	{
		WCHAR wszBadFile[MAX_PATH];
		WCHAR wszErrorMsg[MAX_PATH+128];
		static char szErrorMsg[MAX_PATH+128];
		GetCurrentDirectoryW( MAX_PATH, wszBadFile );
		lstrcatW( wszBadFile, L"\\" );
		lstrcatW( wszBadFile, FileName );
		swprintf( wszErrorMsg, L"Cannot find '%s'", wszBadFile );
		
		// Convert to MBCS
		WideCharToMultiByte( CP_ACP, 0, wszErrorMsg, -1, szErrorMsg, MAX_PATH+128, NULL, NULL );
		
		m_ErrorString = szErrorMsg;
		return false;
	}
	
	//
	// Read file in
	//
	DWORD Size=GetFileSize( hFile, NULL );
	DWORD Len;
	m_pchFile= new char[Size+32];						// Some extra room for string compares
	BOOL bSuccess = ReadFile( hFile, m_pchFile, Size, &Len, NULL );
	CloseHandle( hFile );
	if( !bSuccess )
	{
		delete [] m_pchFile;
		return false;
	}
	
	m_pchFile[Size]=13; 									// Make sure last line ends with CR.
	m_pchCurrent=m_pchFile;
	m_pchCurrentLine=m_pchCurrent;
	m_LineNumber=1;
	m_pchEndOfFile=m_pchFile+Size;
	
	if (!DoRequirements())
	{
		delete [] m_pchFile;
		return false;
	}
	
	m_pchCurrent=m_pchFile;
	m_pchCurrentLine=m_pchCurrent;
	m_LineNumber=1;
	if (!DoPropertySets())
	{
		delete [] m_pchFile;
		return false;
	}
	
	if (!DoPreApplyToAll())
	{
		delete [] m_pchFile;
		return false;
	}
	
	// Save the location marker after pre-ApplyToAll block.
	char *pchAfterPreApplyToAll = m_pchCurrent;
	char *pchLineAfterPreApplyToAll = m_pchCurrentLine;
	int nLineNumberAfterPreApplyToAll = m_LineNumber;
	if (!DoDisplayVendorAndDevice())
	{
		delete [] m_pchFile;
		return false;
	}
	// Save the location marker after display vendor section
	// to help parse the post-applytoall block.
	char *pchAfterDisplayVendor = m_pchCurrent;
	char *pchLineAfterDisplayVendor = m_pchCurrentLine;
	DWORD nLineNumberAfterDisplayVendor = m_LineNumber;
	
	// Restore text pointers to the location after the pre-
	// ApplyToAll block.
	m_pchCurrent = pchAfterPreApplyToAll;
	m_pchCurrentLine = pchLineAfterPreApplyToAll;
	m_LineNumber = nLineNumberAfterPreApplyToAll;
	
	// Start post-ApplyToAll parsing at the end of
	// display section.
	m_pchCurrent = max( m_pchCurrent, pchAfterDisplayVendor );
	m_pchCurrentLine = max( m_pchCurrentLine, pchLineAfterDisplayVendor );
	m_LineNumber = max( m_LineNumber, nLineNumberAfterDisplayVendor );
	if (!DoPostApplyToAll())
	{
		delete [] m_pchFile;
		return false;
	}
	
	//
	// Clean up and retrun
	delete [] m_pchFile;
	return true;
}


CDXConfibDBImpl::CDXConfibDBImpl()
:	m_pDevice(NULL),
	m_pRequirements(NULL)
{
}

CDXConfibDBImpl::~CDXConfibDBImpl()
{
	delete m_pDevice;
	delete m_pRequirements;
	
	std::map<std::string, CPropertySet*>::iterator it, end;
	it = m_mapNameToPropertySet.begin();
	end = m_mapNameToPropertySet.end();
	while( it != end ) {
		delete (*it).second;
		++it;
	}
}

bool CDXConfibDBImpl::DoPreApplyToAll() {
	do {
		if( NextStringIs(kDBKwApplyToAll) ) {
			SkipToNextLine();
			if( !DoPropertySet(m_pDevice) )
				return false;
		} else if( NextStringIs(kDBKwDisplayVendor) )
			break;
		
		SkipToNextLine();
		
	} while( m_pchCurrent < m_pchEndOfFile );
	
	return true;
}

bool CDXConfibDBImpl::DoPostApplyToAll()
{
	do {
		if( NextStringIs(kDBKwApplyToAll) ) {
			SkipToNextLine();
			if( !DoPropertySet(m_pDevice) )
				return false;
		}
		SkipToNextLine();
	} while( m_pchCurrent < m_pchEndOfFile );
	
	return true;
}


bool CDXConfibDBImpl::DoPropertySets()
{
	do {
		if( NextStringIs(kDBKwPropertySet) ) {
			m_pchCurrent+=11;
			
			SkipSpace();
			
			if( *m_pchCurrent=='=' ) {
				m_pchCurrent++;
				
				char* pszName = GetString();
				if (!pszName) {
					return false;
				}
				
				std::string strName = pszName;
				
				SkipToNextLine();
				CPropertySet* pSet = new CPropertySet(*this);
				if (!DoPropertySet(pSet)) {
					delete pSet;
					return false;
				}
				
				m_mapNameToPropertySet[strName] = pSet;
				
			} else {
				SyntaxError("Missing =");
				return false;
			}
		}
		else if( NextStringIs(kDBKwDisplayVendor) || NextStringIs(kDBKwApplyToAll) )
			break;
		
		SkipToNextLine();
		
	} while( m_pchCurrent < m_pchEndOfFile );
	
	return true;
}

bool CDXConfibDBImpl::DoDisplayVendorAndDevice()
{
	//
	// Find correct VendorID
	//
	DWORD Vendor = (DWORD)-1;
	do {
		if( NextStringIs(kDBKwDisplayVendor) ) {
			m_pchCurrent+=13;
			
			SkipSpace();
			
			if( *m_pchCurrent=='=' ) {
				m_pchCurrent++;
				
				if( NextStringIs(kDBKwUnknown) ) {
					Vendor=mCaps.getAdapterID().VendorId;
					break;
				}
				
				DWORD tempVendor=GetNumber();						// Will be -1 if error
				
				if( tempVendor==mCaps.getAdapterID().VendorId ) {
					Vendor=tempVendor;
					break;
				}
			}
		} else if( NextStringIs(kDBKwApplyToAll) ) {
			break;
		}
		
		SkipToNextLine();
		
	} while( m_pchCurrent < m_pchEndOfFile );
	
	//
	// Deal with Unknown VendorID
	if( Vendor==-1 ) {
		return true;
	}
	
	//
	// Get Vendor Name
	const char* pszVendor = GetString();
	if( !pszVendor ) {
		return false;
	}
	m_VendorString = pszVendor;
	
	//
	// Skip over any other vendor names (Some devices have multiple vendors)
	do {
		SkipToNextLine();
		
		if( !NextStringIs(kDBKwDisplayVendor) ) {
			SkipSpace();
			
			if( *m_pchCurrent>='0' && *m_pchCurrent<='9' )
				break;
			
			if( *m_pchCurrent!=13 && *(WORD*)m_pchCurrent!='//' ) {		// Ignore comments and blank lines
				// If we find a line that is not a comment or vendor before
				// the first device, it must be the start of a property set.
				if( !DoPropertySet(m_pDevice) )
					return false;
			}
		}
		
	} while( m_pchCurrent < m_pchEndOfFile );
	
	//
	// Now search for DeviceID	(Until next vendor or end of file is found)
	DWORD DeviceID = (DWORD)-1;
	do {
		if( NextStringIs(kDBKwDisplayVendor) )
			break;
		
		if( NextStringIs(kDBKwUnknown) ) {
			DeviceID=mCaps.getAdapterID().DeviceId;
			m_pchCurrent += strlen (kDBKwUnknown);
			SkipSpace();
			break;
		}
		
		SkipSpace();
		
		if( *m_pchCurrent>='0' && *m_pchCurrent<='9' ) {
			DWORD tempDeviceID=GetNumber();
			
			if( tempDeviceID==mCaps.getAdapterID().DeviceId ) {
				DeviceID=tempDeviceID;
				break;
			}
		}
		
		SkipToNextLine();
		
	} while( m_pchCurrent < m_pchEndOfFile );
	
	//
	// Deal with unknown DeviceID
	if( DeviceID==-1 )
		return true;
	
	//
	// Get Device Name
	if( *m_pchCurrent!='=' ) {
		SyntaxError( "xxx = Device Name expected" );
		return false;
	}
	
	//
	// Add the device name
	m_pchCurrent++;
	
	char* pszDevice = GetString();
	if (!pszDevice) {
		return false;
	}
	m_DeviceString = pszDevice;
	
	//
	// Now add flags until break is found
	SkipToNextLine();
	if( !DoPropertySet(m_pDevice) )
		return false;
	
	return true;
}


bool CDXConfibDBImpl::DoRequirements()
{
	//
	// Find correct Requirements section
	do {
		if( NextStringIs(kDBKwRequirements) )
			break;
		SkipToNextLine();
		
	} while( m_pchCurrent < m_pchEndOfFile );
	
	//
	// No Requirment section?
	if( m_pchCurrent>=m_pchEndOfFile )
		return true;
	
	//
	// Get Requirements
	SkipToNextLine();
	if( !DoPropertySet(m_pRequirements) )
		return false;
	
	return true;
}

bool CDXConfibDBImpl::DoPropertySet( CPropertySet* pPropertySet )
{
	//
	// Now add flags until break is found
	const int k_MaxNestedIf = 256;	// Max levels of nesting
	int NestedIf[k_MaxNestedIf];
	int IfPointer=0;				// Depth of IF's
	int SkippingIF=0;				// 0=Not in an IF, 1=In True if, 2=In false IF
	bool first = true;
	
	do {
		if( first )
			first = false;
		else
			SkipToNextLine();
		
		//
		// Check for unexpected keywords
		if( NextStringIs(kDBKwDisplayVendor) || NextStringIs(kDBKwRequirements) )
			return true;
		
		SkipSpace();
		
		if( *m_pchCurrent!=13 && *(WORD*)m_pchCurrent!='//' ) {		// Ignore comments and blank lines
			if( *m_pchCurrent>='0' && *m_pchCurrent<='9' )		// Or the start of another device
				continue;
			
			if( NextStringIs(kDBKwUnknown) ) 			// Or unknown device clause
				continue;
			
			if( NextStringIs(kDBKwBreak) )
				break;
			
			if( NextStringIs(kDBKwMaxOverallGraphicDetail) ) {
				m_pchCurrent += strlen(kDBKwMaxOverallGraphicDetail);
				
				//	This should be in the format: MaxOGD = N
				//	Skip spaces, '='s.
				SkipSpace();
				if (*m_pchCurrent == '=')
				{
					m_pchCurrent++;
				}
				else
				{
					SyntaxError("Expecting \'=\', didn\'t get it");
					return false;
				}
				SkipSpace();
				
				//	We have the value in string format in m_pchCurrent and we need the
				//	value in both string and numeric format. Doing a GetNumber then a
				//	sprintf works best because it will keep extra spaces or comments out
				//	of the string version.
				
				DWORD dwMaxOGD = GetNumber();
				if (dwMaxOGD != (DWORD)-1)
				{
					char maxValue[16];
					char curValue[16] = "0";
					
					sprintf(maxValue, "%d", dwMaxOGD);
					if (m_pDevice->Get(kDBKwOverallGraphicDetail, curValue, sizeof(curValue)))
					{
						DWORD dwOGD = atoi(curValue);
						if (dwOGD > dwMaxOGD)
						{
							//	Check the current graphic detail level. If its
							//	higher than the max, apply the max.
							std::map<std::string, CPropertySet*>::iterator it;
							it = m_mapOGDToPropertySet.find(maxValue);
							if (it == m_mapOGDToPropertySet.end())
							{
								SyntaxError("Unrecognized graphic detail");
								return false;
							}
							CPropertySet* pSet = (*it).second;
							ApplyPropertySet(m_pDevice, pSet);
						}
					}
				}
				else
				{
					SyntaxError("MaxOverallGraphicDetail did not specify a number!");
					return false;
				}
				
				continue;
			}
			
			if (NextStringIs(kDBKwIf))
			{
				m_pchCurrent+=2;
				
				char* condition=GetCondition();
				
				NestedIf[IfPointer++]=SkippingIF;
				
				if( IfPointer==16 )
				{
					SyntaxError( "IF's nested too deep" );
					return false;
				}
				
				if( condition==(char*)1 )						// True?
				{
					if( SkippingIF!=2 )
					{
						SkippingIF=1;							// If inside false if, keep skipping
					}
				}
				else
				{
					if( condition==(char*)0 )					// False?
					{
						SkippingIF=2;
					}
					else
					{
						if( condition ) 						// m_fError
						{
							SyntaxError( condition );
							return false;
						}
					}
				}
			}
			else
			{
				if (NextStringIs(kDBKwEndif))
				{
					if( IfPointer==0 )
					{
						SyntaxError( "Unexpected ENDIF" );
						return false;
					}
					
					SkippingIF=NestedIf[--IfPointer];
				}
				else
				{
					if( 0==SkippingIF || 1==SkippingIF )					// Add flag if not skipping in a IF
					{
						if ( NextStringIs(kDBKwPropertySet))
						{
							m_pchCurrent+=11;
							
							SkipSpace();
							
							if ( *m_pchCurrent=='=' )
							{
								m_pchCurrent++;
								
								char* pszName = GetString();
								if (!pszName)
								{
									return false;
								}
								std::string strName = pszName;
								
								std::map<std::string, CPropertySet*>::iterator it;
								it = m_mapNameToPropertySet.find(strName);
								if (it == m_mapNameToPropertySet.end())
								{
									SyntaxError("Unrecognized property set");
									return false;
								}
								CPropertySet* pSet = (*it).second;
								ApplyPropertySet(pPropertySet, pSet);
							}
							else
							{
								SyntaxError("Missing =");
								return false;
							}
						}
						else
						{
							char* strErr=AddFlag(pPropertySet);  // Add this flag, check for duplicates
							
							if (strErr)
							{
								SyntaxError( strErr );
								return false;
							}
						}
					}
				}
			}
		}
		
	} while( m_pchCurrent<m_pchEndOfFile );
	
	//
	// Check for hanging endif
	//
	if( SkippingIF!=0 || IfPointer!=0 )
	{
		SyntaxError( "Bad IF/ENDIF" );
		return false;
	}
	
	return true;
}

void CDXConfibDBImpl::ApplyPropertySet(CPropertySet* pSetDst, CPropertySet* pSetSrc)
{
	std::vector<StringPair>::const_iterator it, end;
	it = pSetSrc->GetProperties().begin();
	end = pSetSrc->GetProperties().end();
	while (it != end)
	{
		const StringPair& pr = *it;
		pSetDst->Set(pr.first.c_str(), pr.second.c_str());
		++it;
	}
}

IDXConfigDB* IDXConfigDB::Create()
{
	CDXConfibDBImpl* pRet = new CDXConfibDBImpl();
	return pRet;
}
