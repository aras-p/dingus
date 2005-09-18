#ifndef __GAME_INFO_H
#define __GAME_INFO_H

#include "DemoResources.h"

class CGameDesc;
class CGameState;
class CLevelMesh;
class CPointsMesh;
class CMinimapRenderer;
class CEntityInfoRenderer;
class CEntityManager;


/// Job sets this to error message, or NULL if no error.
extern std::string	gErrorMsg;


/// App settings
extern SAppSettings gAppSettings;


class CGameInfo : public boost::noncopyable {
public:
	// Singleton
	static void initialize( const std::string& server, int port, HWND wnd );
	static CGameInfo& getInstance() { assert(mSingleInstance); return *mSingleInstance; }
	static void finalize();

	/**
	 *  Multi-step initialization start. Returns the first step's name.
	 */
	const char* initBegin();

	/**
	 *  Multi-step initialization. Performs a step and returns next step's
	 *  description.
	 *  @return Step description or NULL if no more steps left.
	 */
	const char* initStep();

	const CGameDesc& getGameDesc() const { return *mGameDesc; }
	CGameDesc& getGameDesc() { return *mGameDesc; }
	const CLevelMesh& getLevelMesh() const { return *mLevelMesh; }
	CLevelMesh& getLevelMesh() { return *mLevelMesh; }
	const CPointsMesh& getPointsMesh() const { return *mPointsMesh; }
	CPointsMesh& getPointsMesh() { return *mPointsMesh; }

	const CGameState& getState() const { return *mState; }
	CGameState& getState() { return *mState; }

	const CEntityManager& getEntities() const { return *mEntities; }
	CEntityManager& getEntities() { return *mEntities; }

	CMinimapRenderer& getMinimapRenderer() { return *mMinimapRenderer; }
	CMinimapRenderer& getEntityBlobsRenderer() { return *mEntityBlobsRenderer; }
	CEntityInfoRenderer& getEntityInfoRenderer() { return *mEntityInfoRenderer; }

	/*
	const CTimer& getTimer() const { return mTimer; }

	float	getTime() const { return mTime; }
	int		getTimeTurn() const { return (int)mTime; }
	void	setTime( float t ) { mTimer.update( time_value::fromsec(t-mTime) ); mTime = t; }
	*/

private:
	static CGameInfo*	mSingleInstance;
	CGameInfo( const std::string& server, int port, HWND wnd );
	~CGameInfo();

private:
	//CTimer			mTimer;

	std::string		mServerName;
	int				mServerPort;
	HWND			mWindow;

	CGameDesc*		mGameDesc;
	CGameState*		mState;
	CLevelMesh*		mLevelMesh;
	CPointsMesh*	mPointsMesh;
	CMinimapRenderer*	mMinimapRenderer;
	CMinimapRenderer*	mEntityBlobsRenderer;
	CEntityInfoRenderer*		mEntityInfoRenderer;
	CEntityManager*		mEntities;

	//float			mTime;
};


#endif
