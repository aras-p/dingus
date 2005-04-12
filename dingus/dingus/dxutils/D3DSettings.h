// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef D3D_SETTINGS_H__
#define D3D_SETTINGS_H__

#include "D3DEnumeration.h"

namespace dingus {

class CD3DApplication;

// --------------------------------------------------------------------------

/**
 *  D3D settings for serialization.
 */
struct SD3DSettingsPref {
	SD3DSettingsPref() { memset( this, -1, sizeof(*this) ); }

	int		adapterVendorID;
	int		adapterDeviceID;
	int		device; // D3DDEVTYPE
	int		mode; // CD3DSettings::eMode
	int		vsync; // -1 or 0/1
	int		fsWidth;
	int		fsHeight;
	int		fsRefresh;
	int		fsFormat; // D3DFORMAT
	int		fsaaType; // D3DMULTISAMPLE_TYPE
	int		fsaaQuality;
	int		backbuffer; // D3DFORMAT
	int		zstencil; // D3DFORMAT
	int		vertexProc; // eVertexProcessing
};


// --------------------------------------------------------------------------

/**
 *  Current D3D settings: adapter, device, mode, formats, etc.
 */
class CD3DSettings {
public:
	enum eMode { WINDOWED = 0, FULLSCREEN, MODE_COUNT };

	struct SSettings {
		const SD3DAdapterInfo*	adapterInfo;
		const SD3DDeviceInfo*	deviceInfo;
		const SD3DDeviceCombo*	deviceCombo;

		D3DDISPLAYMODE		displayMode;
		D3DFORMAT			depthStencilFormat;
		D3DMULTISAMPLE_TYPE multisampleType;
		DWORD				multisampleQuality;
		CD3DDeviceCaps::eVertexProcessing vertexProcessing;
		bool				vsync;
	};
public:
	eMode			mMode;

	SSettings		mSettings[MODE_COUNT];

	int		mWindowedWidth;
	int		mWindowedHeight;

public:
	bool operator==( const CD3DSettings& r ) const { return memcmp( this, &r, sizeof(r) ) ? false : true; }
	bool operator!=( const CD3DSettings& r ) const { return memcmp( this, &r, sizeof(r) ) ? true : false; }
	
    const SD3DAdapterInfo& getAdapterInfo() const { return *mSettings[mMode].adapterInfo; }
    const SD3DDeviceInfo& getDeviceInfo() const { return *mSettings[mMode].deviceInfo; }
    const SD3DDeviceCombo& getDeviceCombo() const { return *mSettings[mMode].deviceCombo; }

    int			getAdapterOrdinal() const { return getDeviceCombo().adapterOrdinal; }
    D3DDEVTYPE	getDevType() const { return getDeviceCombo().deviceType; }
    D3DFORMAT	getBackBufferFormat() const { return getDeviceCombo().backBufferFormat; }

    const D3DDISPLAYMODE&	getDisplayMode() const { return mSettings[mMode].displayMode; }
    D3DDISPLAYMODE&	getDisplayMode() { return mSettings[mMode].displayMode; }
    void			setDisplayMode( D3DDISPLAYMODE dm ) { mSettings[mMode].displayMode = dm; }

    D3DFORMAT		getDepthStencilFormat() const { return mSettings[mMode].depthStencilFormat; }
    void			setDepthStencilFormat( D3DFORMAT fmt ) { mSettings[mMode].depthStencilFormat = fmt; }

    D3DMULTISAMPLE_TYPE	getMultiSampleType() const { return mSettings[mMode].multisampleType; }
    void				setMultiSampleType( D3DMULTISAMPLE_TYPE ms ) { mSettings[mMode].multisampleType = ms; }

    DWORD	getMultiSampleQuality() const { return mSettings[mMode].multisampleQuality; }
    void	setMultiSampleQuality( DWORD q ) { mSettings[mMode].multisampleQuality = q; }

    CD3DDeviceCaps::eVertexProcessing getVertexProcessing() const { return mSettings[mMode].vertexProcessing; }
    void				setVertexProcessing( CD3DDeviceCaps::eVertexProcessing vp ) { mSettings[mMode].vertexProcessing = vp; }

    bool	isVSync() const { return mSettings[mMode].vsync; }
    void	setVSync( bool vs ) { mSettings[mMode].vsync = vs; }
};


// --------------------------------------------------------------------------

/**
 *  Abstract D3D settings dialog.
 */
class CAbstractD3DSettingsDialog {
public:
    CAbstractD3DSettingsDialog( const CD3DEnumeration& enumeration, const CD3DSettings& settings );
	virtual ~CAbstractD3DSettingsDialog() { }
    void		getFinalSettings( CD3DSettings& settings ) { settings = mSettings; }

protected:
    CD3DSettings		mSettings;
    const CD3DEnumeration*	mEnumeration;

protected:
	virtual void	enableWindow( int id, bool enabled ) = 0;
	virtual void	checkButton( int id, bool checked ) = 0;
	virtual bool	isChecked( int id ) = 0;
    virtual void	comboBoxAdd( int id, const void* pData, const TCHAR* pstrDesc ) = 0;
    virtual void	comboBoxSelect( int id, const void* pData ) = 0;
    virtual const void*	comboBoxSelected( int id ) = 0;
    virtual bool	comboBoxSomethingSelected( int id ) = 0;
    virtual int		comboBoxCount( int id ) = 0;
    virtual void	comboBoxSelectIndex( int id, int index ) = 0;
    virtual void	comboBoxClear( int id ) = 0;
    virtual bool	comboBoxContainsText( int id, const TCHAR* pstrText ) = 0;

    void	adapterChanged();
    void	deviceChanged();
    void	windowedFullscreenChanged();
    void	adapterFormatChanged();
    void	resolutionChanged();
    void	refreshRateChanged();
    void	backBufferFormatChanged();
    void	depthStencilBufferChanged();
    void	multiSampleTypeChanged();
    void	multiSampleQualityChanged();
    void	vertexProcessingChanged();
    void	vsyncChanged();
};


// --------------------------------------------------------------------------

/**
 *  Win32 dialog box to allow change the D3D settings.
 */
class CD3DSettingsDialogWin32 : public CAbstractD3DSettingsDialog {
public:
    CD3DSettingsDialogWin32( const CD3DEnumeration& enumeration, const CD3DSettings& settings );
    INT_PTR		showDialog( HINSTANCE instance, HWND hwndParent, CD3DApplication& application );
    INT_PTR		dialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

private:
    HWND				mDlg;

protected:
	virtual void	enableWindow( int id, bool enabled );
	virtual void	checkButton( int id, bool checked );
	virtual bool	isChecked( int id );
    virtual void	comboBoxAdd( int id, const void* pData, const TCHAR* pstrDesc );
    virtual void	comboBoxSelect( int id, const void* pData );
    virtual const void*	comboBoxSelected( int id );
    virtual bool	comboBoxSomethingSelected( int id );
    virtual int		comboBoxCount( int id );
    virtual void	comboBoxSelectIndex( int id, int index );
    virtual void	comboBoxClear( int id );
    virtual bool	comboBoxContainsText( int id, const TCHAR* pstrText );
};


}; // namespace dingus


#endif



