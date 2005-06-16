
#ifndef __KERNEL_SYSTEM_H
#define __KERNEL_SYSTEM_H

#include <dingus/app/DingusApplication.h>

namespace dingus {
	class IConsoleRenderingContext;
	class CD3DTextBoxConsoleRenderingContext;
	class CD3DFont;
};


class CSystem : public IDingusApplication {
public:
	virtual SAppStartupParams getStartupParams();

	virtual IConsoleRenderingContext* createStdConsoleCtx( HWND hwnd );

	virtual void setupBundles( const std::string& dataPath, dingus::CReloadableBundleManager& reloadManager );
	virtual void setupContexts( HWND hwnd );
	virtual void destroyContexts();
	virtual void destroyBundles();

protected:
	HWND	mHwnd;
};


#endif
