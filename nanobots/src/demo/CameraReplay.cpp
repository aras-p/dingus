#include "stdafx.h"
#include "CameraReplay.h"
#include <dingus/math/Interpolation.h>

const int CAMERA_REPLAY_VERSION = 20060416;

CCameraReplay::CCameraReplay( const std::string& fileName, eRecordMode mode )
:	mFile(NULL)
,	mMode(mode)
{
	if( mode != REC_NONE )
		mFile = fopen( fileName.c_str(), mMode == REC_WRITE ? "wt" : "rt" );
	if( !mFile )
		mMode = REC_NONE;

	if( mMode == REC_WRITE )
	{
		writeReplayHeader();
	}
	else if( mMode == REC_READ )
	{
		if( !readReplayFile() )
			mMode = REC_NONE;
	}
}

CCameraReplay::~CCameraReplay()
{
	if( mFile )
		fclose( mFile );
}

void CCameraReplay::SFrameData::lerp( const SFrameData& a, const SFrameData& b, float t )
{
	pos = math_lerp<SVector3>( a.pos, b.pos, t );
	rot.nlerp( a.rot, b.rot, t );
	megaZoom = math_lerp<float>( a.megaZoom, b.megaZoom, t );
	megaTilt = math_lerp<float>( a.megaTilt, b.megaTilt, t );
	followMode = a.followMode;
}

static void writeFrameDataToFile( FILE* f, int frame, const CCameraReplay::SFrameData& d )
{
	static int prevFrame = 0;
	static CCameraReplay::SFrameData prevData;

	if( frame == prevFrame )
	{
		prevData = d;
		return;
	}

	fprintf( f, "%i pos %f %f %f rot %f %f %f %f zoom %f tilt %f follow %i\n",
		prevFrame,
		prevData.pos.x, prevData.pos.y, prevData.pos.z,
		prevData.rot.x, prevData.rot.y, prevData.rot.z, prevData.rot.w, 
		prevData.megaZoom, prevData.megaTilt, prevData.followMode );

	prevFrame = frame;
	prevData = d;
}

static bool readFrameDataFromFile( FILE* f, int& frame, CCameraReplay::SFrameData& d )
{
	int followMode;
	int count = fscanf( f, "%i pos %f %f %f rot %f %f %f %f zoom %f tilt %f follow %i\n",
		&frame,
		&d.pos.x, &d.pos.y, &d.pos.z,
		&d.rot.x, &d.rot.y, &d.rot.z, &d.rot.w, 
		&d.megaZoom, &d.megaTilt, &followMode );
	d.followMode = followMode ? true : false;

	return count == 11;
}

void CCameraReplay::writeReplayHeader()
{
	fprintf( mFile, "Replay version %i\n", CAMERA_REPLAY_VERSION );
}

bool CCameraReplay::readReplayFile()
{
	int replayVersion = 0;
	fscanf( mFile, "Replay version %i\n", &replayVersion );
	if( replayVersion != CAMERA_REPLAY_VERSION )
	{
		CONS << "Camera replay file version " << replayVersion << " does not match required version " << CAMERA_REPLAY_VERSION << endl;
		CONS << "Ignoring camera replay file" << endl;
		return false;
	}

	mData.clear();
	mData.reserve( 256 );

	while( !feof( mFile ) )
	{
		int frame;
		SFrameData frameData;
		if( !readFrameDataFromFile( mFile, frame, frameData ) )
			return false;

		while( mData.size() != frame+1 )
			mData.push_back( frameData );
	}

	return true;
}

void CCameraReplay::processFrame( float frame, SMatrix4x4& viewerMat, float& zoom, float& tilt, bool& follow )
{
	if( mMode == REC_NONE )
		return;

	int iframe0 = (int)frame;
	int iframe1 = iframe0 + 1;
	float alpha = frame - iframe0;

	if( mMode == REC_READ )
	{
		if( iframe0 >= mData.size() ) iframe0 = mData.size()-1;
		if( iframe1 >= mData.size() ) iframe1 = mData.size()-1;

		SFrameData fd;
		fd.lerp( mData[iframe0], mData[iframe1], alpha );

		viewerMat = SMatrix4x4( fd.pos, fd.rot );
		zoom = fd.megaZoom;
		tilt = fd.megaTilt;
		follow = fd.followMode;
	}
	else
	{
		SFrameData fd;
		fd.pos = viewerMat.getOrigin();
		D3DXQuaternionRotationMatrix( &fd.rot, &viewerMat );
		fd.megaZoom = zoom;
		fd.megaTilt = tilt;
		fd.followMode = follow;

		writeFrameDataToFile( mFile, iframe0, fd );
	}
}
