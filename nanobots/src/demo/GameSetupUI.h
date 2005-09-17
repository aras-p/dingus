#ifndef __GAME_SETUP_UI_H
#define __GAME_SETUP_UI_H


class CGameSetupUI {
public:
	CGameSetupUI();
	~CGameSetupUI();
	
	bool	isFinished() const { return mFinished; }

	void	updateViewer( SMatrix4x4& viewer, float& tilt, float& zoom );

private:
	bool	mFinished;
};


#endif
