// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DINGUS_SYSTEM_H
#define __DINGUS_SYSTEM_H

#include "../dxutils/D3DApplication.h"
#include "DingusApplication.h"

namespace dingus {


class CReloadableBundleManager;
class CD3DFont;
class IConsoleRenderingContext;
class CConsoleChannel;
class CD3DTextBoxConsoleRenderingContext;


class CDingusSystem : public CD3DApplication, public IDingusAppContext {
public:
	CDingusSystem( IDingusApplication& application );
	virtual ~CDingusSystem();

	LRESULT msgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	// IDingusAppContext
	virtual const CD3DEnumeration&	getD3DEnumeration() const;
	virtual const CD3DSettings&		getD3DSettings() const;
	virtual void applyD3DSettings( const CD3DSettings& s );

protected:
	virtual HRESULT initialize();
	virtual HRESULT createDeviceObjects();
	virtual HRESULT activateDeviceObjects();
	virtual HRESULT passivateDeviceObjects();
	virtual HRESULT deleteDeviceObjects();
	virtual HRESULT performOneTime();
	virtual HRESULT shutdown();
	virtual bool checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors );
	virtual HRESULT chooseInitialD3DSettings();


private:
	IDingusApplication*			mApplication;
	std::string					mDataPath;
	CReloadableBundleManager*	mReloadableManager;

	CD3DFont*					mFont;
	IConsoleRenderingContext*	mStdConsoleCtx;
	CD3DTextBoxConsoleRenderingContext* mD3DConsoleCtx;
	
	bool	mAppInited;
};



}; // namespace

#endif
