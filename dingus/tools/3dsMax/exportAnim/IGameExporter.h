
#ifndef __GAME_EXPORTER_H
#define __GAME_EXPORTER_H

#include "globals.h"
#include "decomp.h"

#include <IGame.h>
#include <IGameObject.h>
#include <IGameProperty.h>
#include <IGameControl.h>
#include <IGameModifier.h>
#include <IConversionManager.h>
#include <IGameError.h>

#include "AnimData.h"

struct SExportOptions {
public:
	enum { OPTIONS_VERSION = 20040730 };
public:
	SExportOptions()
	:	mUnitMultiplier(0.001f),
		mFramesPerSample(1),
		mTolPos(0.001f), mTolRot(0.001f), mTolScale(0.01f),
		mLooping(1), mDiscardLastSample(0),
		mDoPos(1), mDoRot(1), mDoScale(0), mDoCamera(0), mDoColor(0),
		mDoLeafs(0), mDoRoots(1), mStripBipFromBones(1),
		mScaleScalar(0), mFlt16Rot(0), mFlt16Scale(0)
	{
	}
public:
	// Multiply units from Max with this.
	float	mUnitMultiplier;
	// Sampling rate
	int		mFramesPerSample;
	// Collapse tolerances
	float	mTolPos, mTolRot, mTolScale;
	// General params
	int		mLooping;
	int		mDiscardLastSample;
	// Filter
	int		mDoPos, mDoRot, mDoScale, mDoCamera, mDoColor;
	int		mDoLeafs, mDoRoots;
	int		mStripBipFromBones;
	// Components options
	int		mScaleScalar; // collapse scale into single float?
	int		mFlt16Rot, mFlt16Scale; // rotation/scale - use 16 bit floats?
};


class IGameExporter : public SceneExport {
public:
	IGameExporter();
	virtual ~IGameExporter();

	// SceneExport interface from 3dsMax SDK
	virtual int 			ExtCount(); 			// Number of extensions supported
	virtual const TCHAR*	Ext(int n); 			// Extension #n (i.e. "3DS")
	virtual const TCHAR*	LongDesc(); 			// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const TCHAR*	ShortDesc();			// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR*	AuthorName();			// ASCII Author name
	virtual const TCHAR*	CopyrightMessage(); 	// ASCII Copyright message
	virtual const TCHAR*	OtherMessage1();		// Other message #1
	virtual const TCHAR*	OtherMessage2();		// Other message #2
	virtual unsigned int	Version();				// Version number * 100 (i.e. v3.01 = 301)
	virtual void			ShowAbout(HWND hWnd);	// Show DLL's "About..." box
	virtual BOOL			SupportsOptions(int ext, DWORD options);
	virtual int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

private:
	void processNode( IGameNode* node );
	void sampleAnim( IGameNode* node );
	
	void dumpMatrix( const GMatrix& m );

	void writeAllData();

	void tryCollapseVec3( TVec3Tab& t, float tol );
	void tryCollapseQuat( TQuatTab& t, float tol );
	void tryCollapseFloat( TFloatTab& t, float tol );
	void tryCollapseColor( TColorTab& t, float tol );

	bool rejectName( const TSTR& name ) const;
	
	BOOL readConfig();
	void writeConfig();
	TSTR getCfgFilename();

public:
	static HWND hParams;
	IGameScene* mGameScene;
	FILE*		mFile;
	
	// export params
	SExportOptions	mOptions;
	TSTR	mNameMustStart;
	TSTR	mNameCantEnd;
	
	int mCurrNodeProgress;
	
	bool mShowPrompts;
	bool mExportSelected;

	TVec3AnimGroup	mAnimPos;
	TQuatAnimGroup	mAnimRot;
	TVec3AnimGroup	mAnimScale;
	TVec3AnimGroup	mAnimCamera;
	TColorAnimGroup	mAnimColor;
	TStringTab		mAnimCurveNames;
	TIntTab			mAnimCurveIDs;
	TIntTab			mAnimCurveParents;

	int			mCurrCurve;
	TVec3Tab	mCurrCurvePos;
	TQuatTab	mCurrCurveRot;
	TVec3Tab	mCurrCurveScale;
	TVec3Tab	mCurrCurveCamera;
	TColorTab	mCurrCurveColor;

	int	mSampleCount;
};


#define IGAMEEXPORTER_CLASS_ID	Class_ID(0x246817b6, 0x200c2eb9)

class IGameExporterClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void*			Create(BOOL loading = FALSE) { return new IGameExporter(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return IGAMEEXPORTER_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName() { return _T("IMAnimExport"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

#endif
