#ifndef __SOUND_WAVE_FORMAT_H
#define __SOUND_WAVE_FORMAT_H

#include <mmsystem.h>
#include <mmreg.h>

namespace dingus {



class CSoundWaveFmt : public boost::noncopyable {
public:
	CSoundWaveFmt();
	~CSoundWaveFmt();
	
	bool open( const std::string& fileName, WAVEFORMATEX* pwfx, bool reading );
	bool openFromMemory( BYTE* data, ULONG dataSize, WAVEFORMATEX* pwfx );
	bool close();
	
	bool read( BYTE* buffer, DWORD sizeToRead, DWORD* sizeRead );
	bool write( DWORD sizeToWrite, BYTE* data, DWORD* sizeWritten );
	
	DWORD	getSize() const { return mSize; }
	bool	resetFile();
	const WAVEFORMATEX* getFormat() const { return mFormat; };

protected:
	bool readMmio();
	bool writeMmio( WAVEFORMATEX *destFmt );

public:
	WAVEFORMATEX* mFormat;
	HMMIO		mMmio;		// MM I/O handle for the WAVE
	MMCKINFO	mChunk;		// Multimedia RIFF chunk
	MMCKINFO	mChunkRiff;	// Use in opening a WAVE file
	DWORD		mSize;		// The size of the wave file
	MMIOINFO	mMmioOut;
	bool		mFromMemory;
	bool		mReading;
	BYTE*		mData;
	BYTE*		mDataCurr;
	ULONG		mDataSize;
};


}; // namespace


#endif
