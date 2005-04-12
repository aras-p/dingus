// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PERLIN_H
#define __PERLIN_H

namespace dingus {


class CPerlin {
public:
	enum { BYTE_BUFFER_SIZE = (1<<16) };
	enum { FLOAT_BUFFER_SIZE = 4096 };
	enum { COS_TABLE_SIZE = 256 };

	enum { OCTAVES = 4 };
public:
	CPerlin();

	/** tileDist <= FLOAT_BUFFER_SIZE! On power of 2, it's normal perlin. */
	float	get1D( float x, int tileDist = 1 ) const;
	/** tileDist <= FLOAT_BUFFER_SIZE! On power of 2, it's normal perlin. */
	float	get2D( float x, float y, int tileDist = 1 ) const;
	/** tileDist <= FLOAT_BUFFER_SIZE! On power of 2, it's normal perlin. */
	float	get3D( float x, float y, float z, int tileDist = 1 ) const;

	unsigned char getByte3D( float x, float y, float z ) const;
	/** 16.16 fixed point */
	unsigned char getByte3DFP( unsigned int x, unsigned int y, unsigned int z ) const;

private:
	enum { BYTE_BUFFER_MASK = BYTE_BUFFER_SIZE-1 };
	enum { FLOAT_BUFFER_MASK = FLOAT_BUFFER_SIZE-1 };
	enum { COS_TABLE_MASK = COS_TABLE_SIZE-1 };
	
private:
	float	mFloatBuffer[FLOAT_BUFFER_SIZE];
	unsigned char mByteBuffer[BYTE_BUFFER_SIZE];
	unsigned char mCosTable[COS_TABLE_SIZE];
};


}; // namespace


#endif
