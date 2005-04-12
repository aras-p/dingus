// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "FontBundle.h"
#include "../utils/Errors.h"
#include "TextureBundle.h"

using namespace dingus;


CFontBundle::CFontBundle()
{
	addExtension( ".dfont" );
};


CGUIFont* CFontBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	// open file
	FILE* f = fopen( fullName.getUniqueName().c_str(), "rb" );
	if( !f ) {
		return NULL;
	}
	assert( f );

	// read magic
	char magic[4];
	fread( &magic, 1, 4, f );
	if( magic[0]!='D' || magic[1]!='F' || magic[2]!='N' || magic[3]!='T' ) {
		std::string msg = "file isn't valid font file! '" + id.getUniqueName() + "'";
		CConsole::CON_ERROR.write( msg );
		THROW_ERROR( msg );
	}
	// letter count
	int letterCount;
	fread( &letterCount, 1, 4, f );
	// first letter
	int firstLetter;
	fread( &firstLetter, 1, 4, f );
	// max width
	unsigned short maxWidth;
	fread( &maxWidth, 1, 2, f );
	// max height
	unsigned short maxHeight;
	fread( &maxHeight, 1, 2, f );

	// load texture - with same name
	CD3DTexture* texture = RGET_TEX(id);
	assert( texture );
	D3DSURFACE_DESC desc;
	texture->getObject()->GetLevelDesc( 0, &desc );
	float halftexX = 0.5f / desc.Width;
	float halftexY = 0.5f / desc.Height;

	// create font
	CGUIFont* font = new CGUIFont( letterCount, firstLetter, maxWidth, maxHeight, *texture );
	assert( font );

	// read letter infos
	for( int i = 0; i < letterCount; ++i ) {
		CGUIFont::SLetter& l = font->getLetterByNumber( i );
		fread( &l.u0, 1, 4, f );
		fread( &l.v0, 1, 4, f );
		fread( &l.u1, 1, 4, f );
		fread( &l.v1, 1, 4, f );
		l.u0 += halftexX;	l.v0 += halftexY;
		l.u1 += halftexX;	l.v1 += halftexY;
		unsigned short v;
		fread( &v, 1, 2, f );
		l.width = v;
		fread( &v, 1, 2, f );
		l.height = v;
	}
	
	// close file
	fclose( f );

	CONSOLE.write( "font loaded '" + id.getUniqueName() + "'" );

	return font;
}
