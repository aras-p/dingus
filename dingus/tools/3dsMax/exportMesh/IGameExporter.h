
#ifndef __GAME_EXPORTER_H
#define __GAME_EXPORTER_H

#include "globals.h"

#include <IGame.h>
#include <IGameObject.h>
#include <IGameProperty.h>
#include <IGameControl.h>
#include <IGameModifier.h>
#include <IConversionManager.h>
#include <IGameError.h>
#include <string>

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9math.h>
#include <d3dx9mesh.h>
#include "MeshProcessor.h"


struct SExportOptions {
public:
	SExportOptions()
	:	mUnitMultiplier(0.001f), //mUseLocalSpace(0),
		mDoPositions(1), mDoNormals(1), mDoTangents(0), mDoBinormals(0),
		mTangentsUseUV(0), mColorEncodeNTB(0),
		mDoSkin(1), mCreate1BoneSkin(1), mStripBipFromBones(1), mSkinBones(4)
	,	mDebugOutput(0)
	{
		mDoUVs[0] = 1;
		for( int i = 1; i < mproc::UV_COUNT; ++i )
			mDoUVs[i] = 0;
	}
public:
	// Multiply units from Max with this.
	float	mUnitMultiplier;
	// Local object space or world space?
	//int		mUseLocalSpace;

	// Mesh components to export
	int		mDoPositions;
	int		mDoNormals, mDoTangents, mDoBinormals;
	int		mTangentsUseUV; // use UVs for tangents (0=base, 1=next active, etc.)
	int		mColorEncodeNTB; // encode normals/tangents/binormals as colors?
	int		mDoUVs[mproc::UV_COUNT]; // base UV, next active, etc.
	
	int		mDoSkin; // export skin weights and skeleton?
	int		mCreate1BoneSkin; // Create 1-bone skin if there's no skinning info?
	int		mStripBipFromBones; // strip "Bip??" from bone names?
	int		mSkinBones; // how many bones/vert

	int		mDebugOutput; // output debug file
};


struct SNodeInfo {
public:
	SNodeInfo( IGameNode& n, IGameMesh& m ) : node(&n), mesh(&m), weights(0), indices(0), bones(0) {
		assert( node && mesh );
	}
	~SNodeInfo() {
		if( weights ) delete[] weights;
		if( indices ) delete[] indices;
		if( bones ) delete bones;
	}

	bool	isSkinned() const {
		return (bones) && (bones->Count() > 0) && (maxBonesPerVert > 0);
	}

	void	createSelfSkin( int meshIndex ) {
		assert( !isSkinned() );
		int		nverts = mesh->GetNumberOfVerts();
		weights = new D3DXVECTOR3[nverts];
		indices = new DWORD[nverts];
		bones = new Tab<IGameNode*>();
		for( int i = 0; i < nverts; ++i ) {
			weights[i].x = 1.0f;
			weights[i].y = 0.0f;
			weights[i].z = 0.0f;
			indices[i] = meshIndex;
		}
		bones->Append( 1, &node );
		maxBonesPerVert = 1;
	}

public:
	// The node
	IGameNode*		node;
	// The mesh object
	IGameMesh*		mesh;
	// These may be NULL if not skinned.
	D3DXVECTOR3*		weights;	// for each vertex
	DWORD*				indices;	// for each vertex
	Tab<IGameNode*>*	bones;
	int					maxBonesPerVert;
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
	typedef Tab<IGameNode*>	TNodeTab;
	typedef Tab<SNodeInfo*>	TNodeInfoTab;

	/// Return NULL on success, msg. otherwise
	const char* gatherNode( IGameNode* node );
	/// Return NULL on success, msg. otherwise
	const char* gatherMesh( SNodeInfo& info );

	/// Return NULL on success, msg. otherwise
	//const char* processNode( IGameNode* node );
	
	void gatherSkin( SNodeInfo& info );

	/// Return NULL on success, msg. otherwise
	const char* meshCreate();
	/// Return NULL on success, msg. otherwise
	const char* meshAddNode( SNodeInfo& info, int& vertOffset, int& triOffset );
	/// Return NULL on success, msg. otherwise
	const char* meshProcess();
	/// Return NULL on success, msg. otherwise
	const char* meshWrite();
	/// Return NULL on success, msg. otherwise
	const char*	cleanupMem();
	
	const char* writeMeshData( ID3DXMesh& mesh, DWORD formatBits );
	const char* writeSkinData( /*IGameSkin& skin, */TNodeTab& bones, int maxBonesPerVert );
	static void reorderBones( TNodeTab& inBones, TNodeTab& outBones );
	static void recurseAddBones( TNodeTab& inBones, int boneIdx, TNodeTab& outBones );

	BOOL readConfig();
	void writeConfig();
	TSTR getCfgFilename();

	void	debugMsg( const char* msg, ... ) const;

public:
	static HWND hParams;
	IGameScene* mGameScene;
	FILE*		mFile;

	SExportOptions	mOptions;
	
	int mCurrNodeProgress;

	IDirect3D9*	mDx;
	IDirect3DDevice9* mDxDevice;

	// Gathered info
	TNodeInfoTab	mNodes;
	int		mTotalVerts;
	int		mTotalTris;
	int		mTotalMaxBonesPerVert;

	// Mesh construction and processing
	//D3DXVECTOR3*	mMeshWeights;
	//DWORD*			mMeshIndices;
	TNodeTab			mMeshBones;
	Tab<IGameMaterial*> mMeshMats;
	mproc::CMesh		mMesh;
	bool				mMeshHasUVs[mproc::UV_COUNT];

	
	bool mShowPrompts;
	bool mExportSelected;

	std::string mDebugFileName;
};


#define IGAMEEXPORTER_CLASS_ID	Class_ID(0x44170490, 0x34fc5fdd)

class IGameExporterClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void*			Create(BOOL loading = FALSE) { return new IGameExporter(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return IGAMEEXPORTER_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName() { return _T("IMMeshExport"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

#endif
