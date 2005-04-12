// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "Perlin.h"
#include "Random.h"


#define YWRAP 16
#define YWRAPBITS 4
#define ZWRAP 256
#define ZWRAPBITS 8


#define COS_INTERPOLATE( i ) ( 0.5f * ( 1.0f - cosf( i * 3.14159265f ) ) )

using namespace dingus;


CPerlin::CPerlin(void)
{
	int i;
	for( i = 0; i < FLOAT_BUFFER_SIZE; i++ )
		mFloatBuffer[i] = gRandom.getFloat();
	for( i = 0; i < BYTE_BUFFER_SIZE; i++ )
		mByteBuffer[i] = (unsigned char)gRandom.getUInt();
	for( i = 0; i < COS_TABLE_SIZE; i++ )
		mCosTable[i] = (unsigned char)(255.0f * COS_INTERPOLATE( i / (float)COS_TABLE_SIZE ));
}


float CPerlin::get1D( float x, int tileDist ) const
{
	int tileMask = tileDist-1;

	if( x < 0 )
		x = -x;
	float	r = 0.0f;
	float	ampl = 0.5f;
	
	int		xi = (int)x;
	float	xf = x - xi;
	
	for( int i = 0; i < OCTAVES; i++ ) {
		int xtile = xi&tileMask;
		int xtile1 = (xi+1)&tileMask;

		float n1;
		n1	= mFloatBuffer[ xtile&FLOAT_BUFFER_MASK ];
		n1 += COS_INTERPOLATE(xf) * ( mFloatBuffer[ xtile1&FLOAT_BUFFER_MASK ] - n1 );
		
		r  += n1 * ampl;

		tileMask <<= 1;
		tileMask |= 1;
		ampl *= 0.5f;
		xi <<= 1;
		xf *= 2.0f;
		if( xf >= 1.0f ) {
			xi++;
			xf -= 1.0f;
		}
	}
	return r;
}


float CPerlin::get2D( float x, float y, int tileDist ) const
{
	int tileMask = tileDist-1;

	if( x<0 )
		x = -x;
	if( y<0 )
		y = -y;
	
	int xi = (int)x, yi = (int)y;
	float xf = x-xi, yf = y-yi;

	float rxf;
	
	float r = 0.0f;
	float ampl = 0.5f;

	for( int i = 0; i < OCTAVES; i++ ) {
		int xtile = xi&tileMask;
		int ytile = yi&tileMask;
		int xtile1 = (xi+1)&tileMask;
		int ytile1 = (yi+1)&tileMask;
		int x0y0 = xtile + (ytile<<YWRAPBITS);
		int x1y0 = xtile1 + (ytile<<YWRAPBITS);
		int x0y1 = xtile + (ytile1<<YWRAPBITS);
		int x1y1 = xtile1 + (ytile1<<YWRAPBITS);

		float n1, n2;
		
		rxf = COS_INTERPOLATE(xf);
		
		n1	= mFloatBuffer[ x0y0&FLOAT_BUFFER_MASK ];
		n1 += rxf * ( mFloatBuffer[ x1y0&FLOAT_BUFFER_MASK ] - n1 );
		n2	= mFloatBuffer[ x0y1&FLOAT_BUFFER_MASK ];
		n2 += rxf * ( mFloatBuffer[ x1y1&FLOAT_BUFFER_MASK ] - n2 );
		n1 += COS_INTERPOLATE(yf) * ( n2 - n1 );
		
		r  += n1 * n1 * ampl;
		
		tileMask <<= 1;
		tileMask |= 1;
		ampl *= 0.5f;
		xf *= 2.0f;	xi<<=1;
		yf *= 2.0f; yi<<=1;
		if( xf >= 1.0f ) {
			xi++;
			xf -= 1.0f;
		}
		if( yf >= 1.0f ) {
			yi++;
			yf -= 1.0f;
		}
	}
	return r;
}

float CPerlin::get3D( float x, float y, float z, int tileDist ) const
{
	int tileMask = tileDist-1;

	if( x < 0 )
		x = -x;
	if( y < 0 )
		y = -y;
	if( z < 0 )
		z = -z;
	
	int xi = (int)x, yi = (int)y, zi = (int)z;
	float xf = x - xi, yf = y - yi, zf = z-zi;
	float rxf, ryf;
	
	float r = 0.0f;
	float ampl = 0.5f;
	
	for( int i = 0; i < OCTAVES; i++ ) {
		int xtile = xi&tileMask;
		int ytile = yi&tileMask;
		int ztile = zi&tileMask;
		int xtile1 = (xi+1)&tileMask;
		int ytile1 = (yi+1)&tileMask;
		int ztile1 = (zi+1)&tileMask;
		int x0y0z0 = xtile + (ytile<<YWRAPBITS) + (ztile<<ZWRAPBITS);
		int x1y0z0 = xtile1 + (ytile<<YWRAPBITS) + (ztile<<ZWRAPBITS);
		int x0y1z0 = xtile + (ytile1<<YWRAPBITS) + (ztile<<ZWRAPBITS);
		int x1y1z0 = xtile1 + (ytile1<<YWRAPBITS) + (ztile<<ZWRAPBITS);
		int x0y0z1 = xtile + (ytile<<YWRAPBITS) + (ztile1<<ZWRAPBITS);
		int x1y0z1 = xtile1 + (ytile<<YWRAPBITS) + (ztile1<<ZWRAPBITS);
		int x0y1z1 = xtile + (ytile1<<YWRAPBITS) + (ztile1<<ZWRAPBITS);
		int x1y1z1 = xtile1 + (ytile1<<YWRAPBITS) + (ztile1<<ZWRAPBITS);

		float n1, n2, n3;
		
		rxf = COS_INTERPOLATE(xf);
		ryf = COS_INTERPOLATE(yf);
		
		n1	= mFloatBuffer[ x0y0z0&FLOAT_BUFFER_MASK ];
		n1 += rxf * ( mFloatBuffer[ x1y0z0&FLOAT_BUFFER_MASK ] - n1 );
		n2	= mFloatBuffer[ x0y1z0&FLOAT_BUFFER_MASK ];
		n2 += rxf * ( mFloatBuffer[ x1y1z0&FLOAT_BUFFER_MASK ] - n2 );
		n1 += ryf * ( n2 - n1 );
		
		n2	= mFloatBuffer[ x0y0z1&FLOAT_BUFFER_MASK ];
		n2 += rxf * ( mFloatBuffer[ x1y0z1&FLOAT_BUFFER_MASK ] - n2 );
		n3	= mFloatBuffer[ x0y1z1&FLOAT_BUFFER_MASK ];
		n3 += rxf * ( mFloatBuffer[ x1y1z1&FLOAT_BUFFER_MASK ] - n3 );
		n2 += ryf * ( n3 - n2 );

		n1 += COS_INTERPOLATE(zf) * ( n2 - n1 );
		
		tileMask <<= 1;
		tileMask |= 1;
		r += n1 * ampl;
		ampl *= 0.5f;
		xi <<= 1; xf *= 2.0f;
		yi <<= 1; yf *= 2.0f;
		zi <<= 1; zf *= 2.0f;
		
		if( xf >= 1.0f ) {
			xi++;
			xf -= 1.0f;
		}
		if( yf >= 1.0f ) {
			yi++;
			yf -= 1.0f;
		}
		if( zf >= 1.0f ) {
			zi++;
			zf -= 1.0f;
		}
	}
	
	return r;
}

unsigned char CPerlin::getByte3DFP( unsigned int xfp, unsigned int yfp, unsigned int zfp ) const
{
	int r = 0;
	
	int amplshift = 17;
	
	// 7 octaves
	while( amplshift < 25 ) {
		unsigned int xindex = (xfp>>16)&BYTE_BUFFER_MASK;
		unsigned int yindex = ((yfp>>16)&BYTE_BUFFER_MASK)<<5; 
		unsigned int zindex = ((zfp>>16)&BYTE_BUFFER_MASK)<<10;
		int xf = mCosTable[ (xfp>>8) & COS_TABLE_MASK ];
		int yf = mCosTable[ (yfp>>8) & COS_TABLE_MASK ];
		int zf = mCosTable[ (zfp>>8) & COS_TABLE_MASK ];
		unsigned int off = xindex + yindex + zindex;
		
		int n1 = mByteBuffer[ off&BYTE_BUFFER_MASK ];
		int n2 = mByteBuffer[ (off+1)&BYTE_BUFFER_MASK ];
		int nh1 = (n1<<8) + (n2-n1) * xf; // 8.8 fixed
		n1 = mByteBuffer[ (off+32)&BYTE_BUFFER_MASK ];
		n2 = mByteBuffer[ (off+33)&BYTE_BUFFER_MASK ];
		int nh2 = (n1<<8) + (n2-n1) * xf; // 8.8 fixed
		int nv1 = ((nh1<<8) + (nh2-nh1) * yf) >> 8; // 8.8 fixed
		
		n1 = mByteBuffer[ (off+1024)&BYTE_BUFFER_MASK ];
		n2 = mByteBuffer[ (off+1025)&BYTE_BUFFER_MASK ];
		nh1 = (n1<<8) + (n2-n1) * xf; // .8
		n1 = mByteBuffer[ (off+1056)&BYTE_BUFFER_MASK ];
		n2 = mByteBuffer[ (off+1057)&BYTE_BUFFER_MASK ];
		nh2 = (n1<<8) + (n2-n1) * xf; // .8
		int nv2 = ((nh1<<8) + (nh2-nh1) * yf) >> 8; // .8		
		
		int n = ((nv1<<8) + (nv2-nv1)*zf) >> amplshift;
		
		r += n;
		xfp <<= 1; yfp <<= 1; zfp <<= 1;		
		amplshift++;
	}	
	
	return unsigned char(r);
}

unsigned char CPerlin::getByte3D( float x, float y, float z ) const
{
	unsigned int xfp = (unsigned int)( x * 65535.0f );
	unsigned int yfp = (unsigned int)( y * 65535.0f );
	unsigned int zfp = (unsigned int)( z * 65535.0f );
	return getByte3DFP( xfp, yfp, zfp );
}
