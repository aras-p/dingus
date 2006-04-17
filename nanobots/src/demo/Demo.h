#ifndef __DEMO_H
#define __DEMO_H

#include "../system/System.h"


class CDemo : public CSystem, public IInputListener {
public:
	CDemo( const std::string& serverName, int serverPort, const std::string& recordFileName, bool writeRecord );

	// IDingusApplication
	virtual bool checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors );
	virtual void initD3DSettingsPref( SD3DSettingsPref& pref );

	virtual void initialize( IDingusAppContext& appContext );
	virtual void shutdown();
	virtual bool shouldFinish();
	virtual bool shouldShowStats();
	virtual void perform();
	virtual bool msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

	// IInputListener
	virtual void onInputEvent( const CInputEvent& event );
	virtual void onInputStage();
};


#endif
