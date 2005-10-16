#include "stdafx.h"
#include "GameMap.h"
#include "../DemoResources.h"
#include "GameColors.h"
#include <dingus/utils/Random.h>
#include <dingus/resource/TextureCreator.h>
#include "../ByteUtils.h"

#include <boost/crc.hpp>

// --------------------------------------------------------------------------

CGameMap::SPoint::SPoint( ePointType atype, int ax, int ay, int d )
: type(atype), x(ax), y(ay), data(d)
{
	switch( type ) {
	case PT_AZN:
		colorMain = gColors.ptAZN.main.c;
		colorTone = gColors.ptAZN.tone.c;
		break;
	case PT_HOSHIMI:
		colorMain = gColors.ptHoshimi.main.c;
		colorTone = gColors.ptHoshimi.tone.c;
		break;
	case PT_INJECTION:
		colorMain = gColors.team[data].main.c;
		colorTone = gColors.team[data].tone.c;
		break;
	case PT_OBJECTIVE:
		colorMain = gColors.ptObjective.main.c;
		colorTone = gColors.ptObjective.tone.c;
		break;
	case PT_DECORATIVE:
		colorMain = 0x00000000;
		colorTone = 0x00000000;
		break;
	default:
		assert( false );
	}
}

// --------------------------------------------------------------------------

class CGameMapTextureCreator : public CFixedTextureCreator {
public:
	CGameMapTextureCreator( const CGameMap& gmap )
	:	CFixedTextureCreator( 256, 256, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED )
	,	mGameMap( &gmap )
	{
	}

	virtual IDirect3DTexture9* createTexture() {
		IDirect3DTexture9* tex = CFixedTextureCreator::createTexture();

		// create the offscreen surface
		HRESULT hr;
		CD3DDevice& dx = CD3DDevice::getInstance();
		IDirect3DSurface9* srcSurf = 0;
		hr = dx.getDevice().CreateOffscreenPlainSurface( mGameMap->getCellsX(), mGameMap->getCellsY(), D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &srcSurf, NULL );
		assert( SUCCEEDED(hr) );

		D3DLOCKED_RECT lr;
		srcSurf->LockRect( &lr, NULL, D3DLOCK_DISCARD );
		const char* linePtr = (const char*)lr.pBits;
		for( int y = 0; y < mGameMap->getCellsY(); ++y ) {
			D3DCOLOR* p = (D3DCOLOR*)linePtr;
			for( int x = 0; x < mGameMap->getCellsX(); ++x ) {
				const CGameMap::SCell& cell = mGameMap->getCell(x,y);
				*p = gColors.minimap[cell.color][cell.type];
				++p;
			}
			linePtr += lr.Pitch;
		}
		srcSurf->UnlockRect();

		// now, filter this into the texture
		IDirect3DSurface9* dstSurf = 0;
		hr = tex->GetSurfaceLevel( 0, &dstSurf );
		assert( SUCCEEDED(hr) );
		hr = D3DXLoadSurfaceFromSurface( dstSurf, NULL, NULL, srcSurf, NULL, NULL, D3DX_FILTER_BOX, 0 );
		dstSurf->Release();
		srcSurf->Release();

		D3DXFilterTexture( tex, NULL, 0, D3DX_FILTER_BOX );
		return tex;
	}

private:
	const CGameMap* mGameMap;
};


// --------------------------------------------------------------------------

CGameMap::CGameMap()
:	mCellsX(0), mCellsY(0), mCells(NULL), mCRC(0)
{
}

CGameMap::~CGameMap()
{
	safeDeleteArray( mCells );
}



std::string CGameMap::initialize()
{
	assert( !mCells );

	/*
	Format of map data:

	string	Tissue Name
	int32	Tissue Bitmap Length
	byte[]	Tissue Bitmap (200*200)
	string	Entites text file
	string	Streams text file
	string	Walls text files
	*/

	const BYTE* data;

	//
	// parse name

	mName = bu::receiveStr();
	// TBD: workaround around Richard's funky stuff
	int lastSlash = mName.find_last_of( "\\//" );
	if( lastSlash >= 0 )
		mName = mName.substr( lastSlash+1, mName.length()-lastSlash );
	CONS << "Game map name: " << mName << endl;

	//
	// read map bitmap

	int bmpSize = bu::receiveInt();
	net::receiveChunk( data, bmpSize, true );
	// compute CRC of the bitmap
	const char* bmpFile = (const char*)data;
	mCRC = boost::crc<32,0xFFFFFFFF,0,0,false,false>( bmpFile, bmpSize );


	HRESULT hr;
	D3DXIMAGE_INFO bitmapInfo;
	hr = D3DXGetImageInfoFromFileInMemory( bmpFile, bmpSize, &bitmapInfo );
	if( FAILED(hr) ) {
		return "Error in game map - incorrect tissue bitmap format";
	}
	assert( bitmapInfo.Width > 10 && bitmapInfo.Height > 10 );
	assert( bitmapInfo.Width * bitmapInfo.Height <= 256*256 );
	if( bitmapInfo.Width < 10 || bitmapInfo.Height < 10 ) {
		return "Error in game map - map is too small";
	}
	if( bitmapInfo.Width * bitmapInfo.Height > 256*256 ) {
		return "Error in game map - map is too large";
	}

	mCellsX = bitmapInfo.Width;
	mCellsY = bitmapInfo.Height;
	mCells = new SCell[ mCellsX * mCellsY ];

	CD3DDevice& dx = CD3DDevice::getInstance();
	IDirect3DSurface9* surface = 0;
	hr = dx.getDevice().CreateOffscreenPlainSurface( bitmapInfo.Width, bitmapInfo.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL );
	assert( SUCCEEDED(hr) );

	hr = D3DXLoadSurfaceFromFileInMemory( surface, NULL, NULL, bmpFile, bmpSize, NULL, D3DX_DEFAULT, 0, NULL );
	if( FAILED(hr) ) {
		return "Error in game map - incorrect cells map format";
	}

	D3DLOCKED_RECT lr;
	surface->LockRect( &lr, NULL, D3DLOCK_READONLY );
	const char* linePtr = (const char*)lr.pBits;
	for( int y = 0; y < mCellsY; ++y ) {
		const D3DCOLOR* p = (const D3DCOLOR*)linePtr;
		for( int x = 0; x < mCellsX; ++x ) {
			SCell& cell = mCells[y*mCellsX+x];
			switch( *p ) {
			case 0xFFff0000:
				cell.type = CELL_BLOOD1;
				cell.color = CCOLOR_BLOOD; // TBD
				break;
			case 0xFF00ff00:
				cell.type = CELL_BLOOD2;
				cell.color = CCOLOR_BONE; // TBD
				break;
			case 0xFF0000ff:
				cell.type = CELL_BLOOD3;
				cell.color = CCOLOR_NEURON; // TBD
				break;
			case 0xFF800000:
				cell.type = CELL_BLOOD1;
				cell.color = CCOLOR_BLOOD; // TBD
				break;
			case 0xFF008000:
				cell.type = CELL_BLOOD2;
				cell.color = CCOLOR_BONE; // TBD
				break;
			case 0xFF000080:
				cell.type = CELL_BLOOD3;
				cell.color = CCOLOR_NEURON; // TBD
				break;
			default:
				cell.type = CELL_BONE;
				cell.color = CCOLOR_BLOOD;
			}
			cell.height = MIN_CELL_HEIGHT;
			cell.nearBone = true;
			++p;
		}
		linePtr += lr.Pitch;
	}
	surface->UnlockRect();
	surface->Release();

	// check map validity
	if( !checkMapValidity() )
		return "Tissue contains invalid topology (cells adjacent only diagonally)";

	//
	// read entities

	std::string pts = bu::receiveStr();
	char* ptsFile = (char*)pts.c_str(); // HACK
	const char* tokens = "\n\r";
	const char* pline = strtok( ptsFile, tokens );
	do {
		if( !pline )
			break;
		int etype, eposx, eposy;
		int fread = sscanf( pline, "%i:%i:%i", &eposx, &eposy, &etype );
		if( fread != 3 )
			break;
		mPoints.push_back( SPoint(ePointType(etype), eposx, eposy ) );
	} while( pline = strtok( NULL, tokens ) );

	//
	// read streams

	std::string strms = bu::receiveStr(); // streams
	char* strmsFile = (char*)strms.c_str(); // HACK
	pline = strtok( strmsFile, tokens );
	do {
		if( !pline )
			break;
		int sx, sy, sw, sh, sdir;
		int fread = sscanf( pline, "%i:%i:%i:%i:%i", &sx, &sy, &sw, &sh, &sdir );
		assert( sx >= 0 && sx < mCellsX );
		assert( sy >= 0 && sy < mCellsY );
		assert( sx+sw <= mCellsX );
		assert( sy+sh <= mCellsY );
		assert( sdir >= 0 && sdir <= 3 );
		assert( fread == 5 );
		if( fread != 5 )
			break;
		SStream strm;
		strm.x = sx;
		strm.y = sy;
		strm.width = sw;
		strm.height = sh;
		switch( sdir ) {
		case 0:	strm.deltaX =  0; strm.deltaY =  1; break;
		case 1:	strm.deltaX =  0; strm.deltaY = -1; break;
		case 2:	strm.deltaX =  1; strm.deltaY =  0; break;
		case 3:	strm.deltaX = -1; strm.deltaY =  0; break;
		}
		mStreams.push_back( strm );
	} while( pline = strtok( NULL, tokens ) );

	// TBD:  walls
	bu::receiveStr(); // walls

	//
	// all is loaded now
	
	// calculate cell heights
	calcCellHeights();

	// add some decorative elements
	static int DECOR_TYPES_IN_TISSUE[DECOR_POINT_TYPE_COUNT] = {
		(1<<CCOLOR_BLOOD), (1<<CCOLOR_BLOOD), (1<<CCOLOR_BLOOD),
		(1<<CCOLOR_BONE), (1<<CCOLOR_BONE), (1<<CCOLOR_BONE) | (1<<CCOLOR_BLOOD),
		(1<<CCOLOR_NEURON), (1<<CCOLOR_NEURON), (1<<CCOLOR_NEURON),
		(1<<CCOLOR_NEURON), (1<<CCOLOR_NEURON), (1<<CCOLOR_NEURON)|(1<<CCOLOR_BLOOD)|(1<<CCOLOR_BONE),
	};
	const int DECOR_PTS_COUNT = 125;
	const int MAX_TRY_COUNT = DECOR_PTS_COUNT * 10;
	int decPtsCounter = 0;
	int tryCounter = 0;
	while( decPtsCounter < DECOR_PTS_COUNT && tryCounter < MAX_TRY_COUNT ) {
		++tryCounter;

		int x = gRandom.getUInt() % mCellsX;
		int y = gRandom.getUInt() % mCellsY;
		const SCell& cell = mCells[pos2index(x,y)];
		if( !isBlood(cell.type) || cell.nearBone )
			continue;
		if( cell.height < 2.0f )
			continue;

		int ptType = gRandom.getUInt() % DECOR_POINT_TYPE_COUNT;
		if( !(DECOR_TYPES_IN_TISSUE[ptType] & (1<<cell.color)) )
			continue;

		mPoints.push_back( SPoint(PT_DECORATIVE,x,y, ptType) );
		decPtsCounter++;
	}

	// register level texture
	CSharedTextureBundle::getInstance().registerTexture( RID_TEX_LEVEL, *new CGameMapTextureCreator( *this ) );

	return ""; // all ok!
}


bool CGameMap::checkMapValidity() const
{
	// invalid map topology is:
	// XO or OX
	// OX    XO
	// i.e. when two blood cells meet only diagonally
	for( int y = 0; y < mCellsY-1; ++y ) {
		for( int x = 0; x < mCellsX-1; ++x ) {
			eCellType t00 = getCell( x, y ).type;
			eCellType t01 = getCell( x, y+1 ).type;
			eCellType t10 = getCell( x+1, y ).type;
			eCellType t11 = getCell( x+1, y+1 ).type;
			if( isBlood(t00) && isBlood(t11) && !isBlood(t01) && !isBlood(t10) )
				return false;
			if( !isBlood(t00) && !isBlood(t11) && isBlood(t01) && isBlood(t10) )
				return false;
		}
	}
	return true;
}


void CGameMap::calcCellHeights()
{
	CRandomFast rnd;

	const int NEIGHBOUR_X[8] = { -1,  0,  1, -1, 1, -1, 0, 1 };
	const int NEIGHBOUR_Y[8] = { -1, -1, -1,  0, 0,  1, 1, 1 };

	int idx = 0;
	for( int y = 0; y < mCellsY; ++y ) {
		for( int x = 0; x < mCellsX; ++x, ++idx ) {
			if( !isBlood( mCells[idx].type ) )
				continue;

			// see if any of 8-neighbors is a bone
			bool nearBone = false;
			if( x < 1 || x >= mCellsX-1 || y < 1 || y >= mCellsY-1 )
				nearBone = true;
			else {
				for( int i = 0; i < 8; ++i ) {
					int nidx = idx + NEIGHBOUR_Y[i]*mCellsX + NEIGHBOUR_X[i];
					if( !isCellBlood(nidx) ) {
						nearBone = true;
						break;
					}
				}
			}
			mCells[idx].nearBone = nearBone;

			const int SAMPLING_R = 20;
			int emptyR = 0;
			for( int r = 1; r < SAMPLING_R; ++r ) {
				const int samples = round( 2*D3DX_PI*r * 0.25f );
				float dphi = 2*D3DX_PI / samples;
				float phi = gRandom.getFloat( dphi );
				bool empty = true;
				for( int i = 0; i < samples; ++i, phi += dphi ) {
					int px = round( x + cosf(phi) * r );
					int py = round( y + sinf(phi) * r );
					if( px < 0 || py < 0 || px >= mCellsX || py >= mCellsY ) {
						empty = false;
						break;
					}
					if( !isCellBlood(py*mCellsX+px) ) {
						empty = false;
						break;
					}
				}
				if( !empty )
					break;
				++emptyR;
			}
			mCells[idx].height += (1-powf(1-(float)emptyR/SAMPLING_R, 2)) * SAMPLING_R * 0.2f;
		}
	}
}
