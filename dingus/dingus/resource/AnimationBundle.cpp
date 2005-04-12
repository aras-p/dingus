// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "AnimationBundle.h"
#include "../animator/AnimDataType.h"
#include "../animator/SampledAnimation.h"

using namespace dingus;

CAnimationBundle::CAnimationBundle()
{
	addExtension( ".danim" );
}

static inline std::string gReadString( FILE* f )
{
	std::string str;
	for(;;) {
		int c = fgetc( f );
		if( c==0 || c==EOF )
			break;
		str += (char)c;
	}
	return str;
}

CAnimationBunch* CAnimationBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	// try open file
	FILE* f = fopen( fullName.getUniqueName().c_str(), "rb" );
	if( !f )
		return NULL;
	assert( f );

	// create bunch
	CAnimationBunch* bunch = new CAnimationBunch( id.getUniqueName() );
	assert( bunch );

	// read magic
	char magic[4];
	fread( &magic, 1, 4, f );
	if( magic[0]!='D' || magic[1]!='A' || magic[2]!='N' || magic[3]!='I' ) {
		std::string msg = "file isn't valid anim file! '" + fullName.getUniqueName() + "'";
		CConsole::CON_ERROR.write( msg );
		THROW_ERROR( msg );
	}
	// read anim type
	int animType;
	fread( &animType, 1, 4, f );
	// loop
	int loopType;
	fread( &loopType, 1, 4, f );
	// curve count
	int curveCount;
	fread( &curveCount, 1, 4, f );
	// group count
	int groupCount;
	fread( &groupCount, 1, 4, f );

	// read curve infos
	for( int i = 0; i < curveCount; ++i ) {
		std::string name = gReadString( f );
		int parent;
		fread( &parent, 1, 4, f );
		bunch->addCurveDesc( name, parent );
	}
	bunch->endCurves();
	
	// read rest based on anim type
	switch( animType ) {
	case 0: // sampled
		readSampledData( *bunch, f, loopType, curveCount, groupCount );
		break;
	default:
		{
			std::string msg = "file contains unknown animation type! '" + fullName.getUniqueName() + "'";
			CConsole::CON_WARNING.write( msg );
			assert( false );
		}
	}

	// close file
	fclose( f );

	CONSOLE.write( "animation loaded '" + id.getUniqueName() + "'" );

	return bunch;
}


template<typename TSRC, typename TDST, typename CONV>
static CSampledAnimation<TDST>*	gReadSampledAnim(
	FILE* f, int loop, int curves, 
	int sampleSize, int samplesPerCurve,
	CONV converter )
{
	typedef CSampledAnimation<TDST>	TAnim;
	typedef CAnimCurve<TDST>			TCurve;
	assert( sampleSize == sizeof(TSRC) );
	TAnim* anim = new TAnim( samplesPerCurve, (TAnim::eLoopType)loop );
	// read curves
	anim->reserveCurves( curves );
	for( int c = 0; c < curves; ++c ) {
		int ipol, firstSample;
		fread( &ipol, 1, 4, f );
		fread( &firstSample, 1, 4, f );
		TSRC collsrc;
		fread( &collsrc, 1, sizeof(collsrc), f );
		TDST colldst;
		converter( 1, &collsrc, &colldst );
		anim->addCurve( TCurve( colldst, firstSample, (TCurve::eIpol)ipol ) );
	}
	// read samples
	int totalSamples;
	fread( &totalSamples, 1, 4, f );
	anim->resizeSamples( totalSamples );
	if( totalSamples > 0 ) {
		// read samples
		TSRC* samples = new TSRC[ totalSamples ];
		fread( samples, totalSamples, sizeof(TSRC), f );
		// convert
		converter( totalSamples, samples, (TDST*)&anim->getSample(0) );
		// delete read samples
		delete[] samples;
	}
	return anim;
}

template<typename T>
struct SAnimDataConverterNone {
	void operator()( int n, const T* src, T* dst ) {
		memcpy( dst, src, n * sizeof(T) );
	}
};

inline void gAnimDataConverterFlt16To32( int n, const D3DXFLOAT16* src, float* dst )
{
	D3DXFloat16To32Array( dst, src, n );
}
inline void gAnimDataConverterVec316To32( int n, const D3DXVECTOR3_16F* src, SVector3* dst )
{
	D3DXFloat16To32Array( *dst, *src, n*3 );
}
inline void gAnimDataConverterQuat16To32( int n, const D3DXVECTOR4_16F* src, SQuaternion* dst )
{
	D3DXFloat16To32Array( *dst, *src, n*4 );
}
inline void gAnimDataConverterColorToVec4( int n, const D3DCOLOR* src, SVector4* dst )
{
	for( int i = 0; i < n; ++i ) {
		D3DXCOLOR c = src[i];
		dst[i].set( c.r, c.g, c.b, c.a );
	}
}


void CAnimationBundle::readSampledData( CAnimationBunch& bunch, FILE* f, int loop, int curves, int groups ) const
{
	// samples per curve
	int samplesPerCurve;
	fread( &samplesPerCurve, 1, 4, f );
	assert( samplesPerCurve > 0 );

	for( int g = 0; g < groups; ++g ) {
		// group type
		int groupType;
		fread( &groupType, 1, 4, f );
		// sample size
		int sampleSize;
		fread( &sampleSize, 1, 4, f );
		// group name
		std::string name = gReadString( f );
		switch( groupType ) {
		case dingus::ANIMTYPE_VEC3:
			bunch.addVector3Anim( name,
				*gReadSampledAnim<SVector3,SVector3>( f, loop, curves, sampleSize, samplesPerCurve,
					SAnimDataConverterNone<SVector3>() ) );
			break;
		case dingus::ANIMTYPE_QUAT:
			bunch.addQuatAnim( name,
				*gReadSampledAnim<SQuaternion,SQuaternion>( f, loop, curves, sampleSize, samplesPerCurve,
					SAnimDataConverterNone<SQuaternion>() ) );
			break;
		case dingus::ANIMTYPE_FLT:
			bunch.addFloatAnim( name,
				*gReadSampledAnim<float,float>( f, loop, curves, sampleSize, samplesPerCurve,
					SAnimDataConverterNone<float>() ) );
			break;
		case dingus::ANIMTYPE_COLOR:
			bunch.addVector4Anim( name,
				*gReadSampledAnim<D3DCOLOR,SVector4>( f, loop, curves, sampleSize, samplesPerCurve,
					gAnimDataConverterColorToVec4 ) );
			break;
		case dingus::ANIMTYPE_VEC3_H:
			bunch.addVector3Anim( name,
				*gReadSampledAnim<D3DXVECTOR3_16F,SVector3>( f, loop, curves, sampleSize, samplesPerCurve,
					gAnimDataConverterVec316To32 ) );
			break;
		case dingus::ANIMTYPE_QUAT_H:
			bunch.addQuatAnim( name,
				*gReadSampledAnim<D3DXVECTOR4_16F,SQuaternion>( f, loop, curves, sampleSize, samplesPerCurve,
					gAnimDataConverterQuat16To32 ) );
			break;
		case dingus::ANIMTYPE_FLT_H:
			bunch.addFloatAnim( name,
				*gReadSampledAnim<D3DXFLOAT16,float>( f, loop, curves, sampleSize, samplesPerCurve,
					gAnimDataConverterFlt16To32 ) );
			break;
		default:
			ASSERT_FAIL_MSG( "unknown animation sample type" );
		}
	}
}
