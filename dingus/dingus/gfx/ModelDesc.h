// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __GFX_MODEL_DESC_H
#define __GFX_MODEL_DESC_H

#include "../renderer/EffectParams.h"
#include "../resource/ResourceId.h"
#include "../utils/MemoryPool.h"

namespace dingus {

class CEffectParams;

// --------------------------------------------------------------------------

class CModelDesc : public boost::noncopyable {
public:
	struct SParams {
	public:
		typedef std::pair<std::string,std::string>	TNameIDPair;
		typedef std::pair<std::string,SVector3>		TNameVec3Pair;
		typedef std::pair<std::string,SVector4>		TNameVec4Pair;
		typedef std::pair<std::string,float>		TNameFloatPair;
		typedef fastvector<TNameIDPair>		TNameIDVector;
		typedef fastvector<TNameVec3Pair>	TNameVec3Vector;
		typedef fastvector<TNameVec4Pair>	TNameVec4Vector;
		typedef fastvector<TNameFloatPair>	TNameFloatVector;
	public:
		SParams() { }
		SParams( const SParams& rhs );
		const SParams& operator=( const SParams& rhs );
	public:
		TNameIDVector		textures;
		TNameIDVector		cubemaps;
		TNameIDVector		stextures;
		TNameVec3Vector		vectors3;
		TNameVec4Vector		vectors4;
		TNameFloatVector	floats;
	};

public:
	CModelDesc( const CResourceId& meshID );

	const CResourceId& getMeshID() const { return mMeshID; }
	void setMeshID( const CResourceId& mid ) { mMeshID = mid; }

	void addGroup( int renderPriority, const CResourceId& fxID );
	void clearGroups() { mGroups.clear(); }

	void	addParamTexture( int group, const std::string& name, const std::string& value );
	void	addParamCubemap( int group, const std::string& name, const std::string& value );
	void	addParamSTexture( int group, const std::string& name, const std::string& value );
	void	addParamVec3( int group, const std::string& name, const SVector3& value );
	void	addParamVec4( int group, const std::string& name, const SVector4& value );
	void	addParamFloat( int group, const std::string& name, float value );

	int		getGroupCount() const;
	const CResourceId& getFxID( int group ) const;
	int		getRenderPriority( int group ) const;
	const SParams& getParams( int group ) const;
	void	fillFxParams( int group, CEffectParams& dest ) const;

private:
	struct SGroup {
	public:
		SGroup( const CResourceId& fx, int rpri ) : fxID(fx), renderPriority(rpri) { }
		SGroup( const SGroup& rhs );
		const SGroup& operator=( const SGroup& rhs );
	public:
		SParams	params;
		CResourceId		fxID;
		int				renderPriority;
	};
	typedef fastvector<SGroup>	TGroupsVector;
private:
	DECLARE_POOLED_ALLOC(dingus::CModelDesc);
private:
	CResourceId		mMeshID;
	TGroupsVector	mGroups;
};


// --------------------------------------------------------------------------

inline CModelDesc::CModelDesc( const CResourceId& meshID )
:	mMeshID( meshID )
{
}

inline int CModelDesc::getGroupCount() const
{
	return mGroups.size();
}

inline int CModelDesc::getRenderPriority( int group ) const
{
	assert(group>=0&&group<getGroupCount());
	return mGroups[group].renderPriority;
}

inline const CResourceId& CModelDesc::getFxID( int group ) const
{
	assert(group>=0&&group<getGroupCount());
	return mGroups[group].fxID;
}

inline const CModelDesc::SParams& CModelDesc::getParams( int group ) const
{
	assert(group>=0&&group<getGroupCount());
	return mGroups[group].params;
}


}; // namespace


#endif
