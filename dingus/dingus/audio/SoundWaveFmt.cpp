#include "stdafx.h"
#include "SoundWaveFmt.h"


using namespace dingus;


CSoundWaveFmt::CSoundWaveFmt()
:	mFormat(NULL), mMmio(NULL),
	mSize(0), mFromMemory(false)
{
}

CSoundWaveFmt::~CSoundWaveFmt()
{
	close();
	if( !mFromMemory )
		safeDeleteArray( mFormat );
}

bool CSoundWaveFmt::open( const std::string& fileName, WAVEFORMATEX* pwfx, bool reading )
{
	mReading = reading;
	mFromMemory = false;
	
	if( reading ) {
		if( fileName.empty() )
			return false;
		safeDeleteArray( mFormat );
		
		mMmio = mmioOpen( (char*)fileName.c_str(), NULL, MMIO_ALLOCBUF | MMIO_READ );
		if( NULL == mMmio )
			return false;
		if( !readMmio() ) {
			mmioClose( mMmio, 0 );
			return false;
		}
		if( !resetFile() )
			return false;
		mSize = mChunk.cksize; // sore the size of the wav file now

	} else {

		mMmio = mmioOpen( (char*)fileName.c_str(), NULL, MMIO_ALLOCBUF	| MMIO_READWRITE | MMIO_CREATE );
		if( NULL == mMmio )
			return false;
		if( !writeMmio( pwfx ) ) {
			mmioClose( mMmio, 0 );
			return false;
		}
		if( !resetFile() )
			return false;
	}
	
	return true;
}

bool CSoundWaveFmt::openFromMemory( BYTE* data, ULONG dataSize, WAVEFORMATEX* pwfx )
{
	mFormat = pwfx;
	mDataSize = dataSize;
	mData = data;
	mDataCurr = mData;
	mFromMemory = true;
	return true;
}

/**
 * Support function for reading from a multimedia I/O stream.
 * mMmio must be valid before calling.  This function uses it to
 * update mChunkRiff and mFormat.
 */
bool CSoundWaveFmt::readMmio()
{
	MMCKINFO		ckIn;			// chunk info for general use.
	PCMWAVEFORMAT	pcmWaveFormat;	// Temp PCM structure to load in.		
	
	mFormat = NULL;
	
	if( ( 0 != mmioDescend( mMmio, &mChunkRiff, NULL, 0 ) ) )
		return false;
	
	// Check to make sure this is a valid wave file
	if( (mChunkRiff.ckid != FOURCC_RIFF) || (mChunkRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
		return false;
	
	// Search the input file for for the 'fmt ' chunk.
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if( 0 != mmioDescend( mMmio, &ckIn, &mChunkRiff, MMIO_FINDCHUNK ) )
		return false;
	
	// Expect the 'fmt' chunk to be at least as large as PCMWAVEFORMAT;
	// if there are extra parameters at the end, we'll ignore them
	if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
		return false;
	
	// read the 'fmt ' chunk into pcmWaveFormat.
	if( mmioRead( mMmio, (HPSTR) &pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
		return false;
	
	// Allocate the waveformatex, but if its not pcm format, read the next
	// word, and thats how many extra bytes to allocate.
	if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM ) {
		mFormat = (WAVEFORMATEX*)(new char[ sizeof(WAVEFORMATEX) ]);
		if( NULL == mFormat )
			return false;
		// Copy the bytes from the pcm structure to the waveformatex structure
		memcpy( mFormat, &pcmWaveFormat, sizeof(pcmWaveFormat) );
		mFormat->cbSize = 0;

	} else {

		// read in length of extra bytes.
		WORD cbExtraBytes = 0L;
		if( mmioRead( mMmio, (char*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
			return false;
		
		mFormat = (WAVEFORMATEX*)(new char[ sizeof(WAVEFORMATEX) + cbExtraBytes ]);
		if( NULL == mFormat )
			return false;
		
		// Copy the bytes from the pcm structure to the waveformatex structure
		memcpy( mFormat, &pcmWaveFormat, sizeof(pcmWaveFormat) );
		mFormat->cbSize = cbExtraBytes;
		
		// Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
		if( mmioRead( mMmio, (char*)(((BYTE*)&(mFormat->cbSize))+sizeof(WORD)), cbExtraBytes ) != cbExtraBytes ) {
			safeDeleteArray( mFormat );
			return false;
		}
	}
	
	// Ascend the input file out of the 'fmt ' chunk.
	if( 0 != mmioAscend( mMmio, &ckIn, 0 ) ) {
		safeDeleteArray( mFormat );
		return false;
	}
	
	return true;
}


/**
 * Resets the internal mChunk pointer so reading starts from
 * the beginning of the file again
 */
bool CSoundWaveFmt::resetFile()
{
	if( mFromMemory ) {
		mDataCurr = mData;
	} else {
		if( mMmio == NULL )
			return false;
		
		if( mReading ) {
			// Seek to the data
			if( -1 == mmioSeek( mMmio, mChunkRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET ) )
				return false;
			
			// Search the input file for the 'data' chunk.
			mChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
			if( 0 != mmioDescend( mMmio, &mChunk, &mChunkRiff, MMIO_FINDCHUNK ) )
				return false;
		} else {
			// Create the 'data' chunk that holds the waveform samples.  
			mChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
			mChunk.cksize = 0;
			if( 0 != mmioCreateChunk( mMmio, &mChunk, 0 ) ) 
				return false;
			if( 0 != mmioGetInfo( mMmio, &mMmioOut, 0 ) )
				return false;
		}
	}
	
	return true;
}

/**
 *  Reads section of data from a wave file into buffer and returns 
 *  how much read in sizeRead, reading not more than sizeToRead.
 *  This uses mChunk to determine where to start reading from.	So
 *  subsequent calls will be continue where the last left off unless
 *  resetFile() is called.
 */
bool CSoundWaveFmt::read( BYTE* buffer, DWORD sizeToRead, DWORD* sizeRead )
{
	if( mFromMemory ) {
		if( mDataCurr == NULL )
			return false;
		if( sizeRead != NULL )
			*sizeRead = 0;
		
		if( mDataCurr + sizeToRead > mData + mDataSize ) {
			sizeToRead = mDataSize - (DWORD)(mDataCurr - mData);
		}
		
		CopyMemory( buffer, mDataCurr, sizeToRead );
	
		if( sizeRead != NULL )
			*sizeRead = sizeToRead;
		
		return true;
	} else {
		MMIOINFO mmioinfoIn; // current status of mMmio
		
		if( mMmio == NULL )
			return false;
		if( buffer == NULL || sizeRead == NULL )
			return false;
		
		if( sizeRead != NULL )
			*sizeRead = 0;
		
		if( 0 != mmioGetInfo( mMmio, &mmioinfoIn, 0 ) )
			return false;
		
		UINT cbDataIn = sizeToRead;
		if( cbDataIn > mChunk.cksize ) 
			cbDataIn = mChunk.cksize;		
		
		mChunk.cksize -= cbDataIn;
		
		for( DWORD cT = 0; cT < cbDataIn; cT++ ) {
			// Copy the bytes from the io to the buffer.
			if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead ) {
				if( 0 != mmioAdvance( mMmio, &mmioinfoIn, MMIO_READ ) )
					return false;
				if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
					return false;
			}
			// Actual copy.
			*((BYTE*)buffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
			mmioinfoIn.pchNext++;
		}
		
		if( 0 != mmioSetInfo( mMmio, &mmioinfoIn, 0 ) )
			return false;
		
		if( sizeRead != NULL )
			*sizeRead = cbDataIn;
		
		return true;
	}
}

bool CSoundWaveFmt::close()
{
	if( mFromMemory )
		return true;

	if( mReading ) {
		mmioClose( mMmio, 0 );
		mMmio = NULL;

	} else {
		mMmioOut.dwFlags |= MMIO_DIRTY;
		if( mMmio == NULL )
			return false;
		
		if( 0 != mmioSetInfo( mMmio, &mMmioOut, 0 ) )
			return false;
		
		// Ascend the output file out of the 'data' chunk -- this will cause
		// the chunk size of the 'data' chunk to be written.
		if( 0 != mmioAscend( mMmio, &mChunk, 0 ) )
			return false;
		// Do this here instead...
		if( 0 != mmioAscend( mMmio, &mChunkRiff, 0 ) )
			return false;
		mmioSeek( mMmio, 0, SEEK_SET );
		if( 0 != (INT)mmioDescend( mMmio, &mChunkRiff, NULL, 0 ) )
			return false;
		
		mChunk.ckid = mmioFOURCC('f', 'a', 'c', 't');
		
		if( 0 == mmioDescend( mMmio, &mChunk, &mChunkRiff, MMIO_FINDCHUNK ) ) {
			DWORD dwSamples = 0;
			mmioWrite( mMmio, (HPSTR)&dwSamples, sizeof(DWORD) );
			mmioAscend( mMmio, &mChunk, 0 ); 
		}
		
		// Ascend the output file out of the 'RIFF' chunk -- this will cause
		// the chunk size of the 'RIFF' chunk to be written.
		if( 0 != mmioAscend( mMmio, &mChunkRiff, 0 ) )
			return false;
		
		mmioClose( mMmio, 0 );
		mMmio = NULL;
	}
	
	return true;
}

/**
 *  Support function for writing to a multimedia I/O stream.
 *  destFmt is the WAVEFORMATEX for this new wave file. 
 *  mMmio must be valid before calling.  This function uses it to
 *  update mChunkRiff and mChunk.	
 */
bool CSoundWaveFmt::writeMmio( WAVEFORMATEX *destFmt )
{
	DWORD	 dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
	MMCKINFO ckOut1;
	
	dwFactChunk = (DWORD)-1;
	
	// Create the output file RIFF chunk of form type 'WAVE'.
	mChunkRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');	   
	mChunkRiff.cksize = 0;
	
	if( 0 != mmioCreateChunk( mMmio, &mChunkRiff, MMIO_CREATERIFF ) )
		return false;
	
	// We are now descended into the 'RIFF' chunk we just created.
	// Now create the 'fmt ' chunk. Since we know the size of this chunk,
	// specify it in the MMCKINFO structure so MMIO doesn't have to seek
	// back and set the chunk size after ascending from the chunk.
	mChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	mChunk.cksize = sizeof(PCMWAVEFORMAT);	 
	
	if( 0 != mmioCreateChunk( mMmio, &mChunk, 0 ) )
		return false;
	
	// write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. 
	if( destFmt->wFormatTag == WAVE_FORMAT_PCM ) {
		if( mmioWrite( mMmio, (HPSTR) destFmt, 
			sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
			return false;
	} else {
		// write the variable length size.
		if( (UINT)mmioWrite( mMmio, (HPSTR) destFmt, 
			sizeof(*destFmt) + destFmt->cbSize ) != 
			( sizeof(*destFmt) + destFmt->cbSize ) )
			return false;
	}  
	
	// Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
	if( 0 != mmioAscend( mMmio, &mChunk, 0 ) )
		return false;
	
	// Now create the fact chunk, not required for PCM but nice to have.  This is filled
	// in when the close routine is called.
	ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
	ckOut1.cksize = 0;
	
	if( 0 != mmioCreateChunk( mMmio, &ckOut1, 0 ) )
		return false;
	
	if( mmioWrite( mMmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != 
		sizeof(dwFactChunk) )
		return false;
	
	// Now ascend out of the fact chunk...
	if( 0 != mmioAscend( mMmio, &ckOut1, 0 ) )
		return false;
	
	return true;
}


/**
 * Writes data to the open wave file
 */
bool CSoundWaveFmt::write( DWORD sizeToWrite, BYTE* data, DWORD* sizeWritten )
{
	DWORD cT;
	
	if( mFromMemory )
		return false;
	if( mMmio == NULL )
		return false;
	if( sizeWritten == NULL || data == NULL )
		return false;
	
	*sizeWritten = 0;
	
	for( cT = 0; cT < sizeToWrite; cT++ ) {		
		if( mMmioOut.pchNext == mMmioOut.pchEndWrite ) {
			mMmioOut.dwFlags |= MMIO_DIRTY;
			if( 0 != mmioAdvance( mMmio, &mMmioOut, MMIO_WRITE ) )
				return false;
		}
		*((BYTE*)mMmioOut.pchNext) = *((BYTE*)data+cT);
		(BYTE*)mMmioOut.pchNext++;
		(*sizeWritten)++;
	}
	
	return true;
}
