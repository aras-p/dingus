#ifndef __DEMO_UI_H
#define __DEMO_UI_H

#include <dingus/dxutils/D3DSettings.h>
#include <dingus/gfx/gui/Gui.h>
#include "DemoResources.h"


// --------------------------------------------------------------------------


class CDemoSettingsDialog : public CAbstractD3DSettingsDialog {
public:
	enum eDlgState { STATE_ACTIVE, STATE_OK, STATE_CANCEL };

public:
    CDemoSettingsDialog( const CD3DEnumeration& enumeration, const CD3DSettings& settings );
	~CDemoSettingsDialog();

	void		showDialog( const CD3DSettings& settings );
	void		hideDialog() { mState = STATE_CANCEL; }
	CUIDialog&	getDialog() { return *mDlg; }
	eDlgState	getState() const { return mState; }

	const SAppSettings& getAppSettings() const { return mAppSettings; }
	SAppSettings& getAppSettings() { return mAppSettings; }

	static void CALLBACK dialogCallback( UINT evt, int ctrlID, CUIControl* ctrl );

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

private:
	static CDemoSettingsDialog*	mSingleInstance;
	
	SAppSettings	mAppSettings;
	CUIDialog*	mDlg;
	eDlgState	mState;
};



// --------------------------------------------------------------------------


class CDemoHelpDialog : public boost::noncopyable {
public:
    CDemoHelpDialog();
	~CDemoHelpDialog();

	void		showDialog();
	void		hideDialog() { mActive = false; }
	CUIDialog&	getDialog() { return *mDlg; }
	bool		isActive() const { return mActive; }

	static void CALLBACK dialogCallback( UINT evt, int ctrlID, CUIControl* ctrl );

private:
	static CDemoHelpDialog*	mSingleInstance;

	CUIDialog*	mDlg;
	bool		mActive;
};



#endif
