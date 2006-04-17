#ifndef __CAMERA_REPLAY_H
#define __CAMERA_REPLAY_H

enum eRecordMode {
	REC_NONE,
	REC_WRITE,
	REC_READ
};


class CCameraReplay
{
public:
	struct SFrameData {
		SVector3	pos;
		SQuaternion	rot;
		float	megaZoom;
		float	megaTilt;
		bool	followMode;

		void	lerp( const SFrameData& a, const SFrameData& b, float t );
	};

public:
	CCameraReplay( const std::string& fileName, eRecordMode mode );
	~CCameraReplay();

	bool	isReadingReplay() const { return mMode == REC_READ; }

	void	processFrame( float frame, SMatrix4x4& viewerMat, float& zoom, float& tilt, bool& follow );

private:
	bool		readReplayFile();
	void		writeReplayHeader();

private:
	FILE*		mFile;
	eRecordMode	mMode;
	std::vector<SFrameData>	mData;
};


#endif
