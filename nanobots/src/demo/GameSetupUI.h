#ifndef __GAME_SETUP_UI_H
#define __GAME_SETUP_UI_H

#include <dingus/gfx/gui/Gui.h>


class CGameSetupDialog : public boost::noncopyable {
public:
	enum eDlgState { STATE_ACTIVE, STATE_START, STATE_EXIT, STATE_HIDDEN };

public:
	CGameSetupDialog();
	~CGameSetupDialog();
	
	//void		showDialog();
	//void		hideDialog() { mState = STATE_HIDDEN; }
	CUIDialog&	getDialog() { return *mDlg; }
	eDlgState	getState() const { return mState; }


	void	render();
	void	updateViewer( SMatrix4x4& viewer, float& tilt, float& zoom );
	//void		update();


	static void CALLBACK dialogCallback( UINT evt, int ctrlID, CUIControl* ctrl );
	
private:
	static CGameSetupDialog*	mSingleInstance;

	CUIDialog*	mDlg;
	eDlgState	mState;
};


#endif
