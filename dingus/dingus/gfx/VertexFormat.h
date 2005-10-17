// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VERTEX_FORMAT_H
#define __VERTEX_FORMAT_H


namespace dingus {


/**
 *  Vertex format descriptor. It's like FVF, but supports more than that.
 *
 *  In the end it's a 4 byte value with bits indicating various vertex components and
 *  their modes.
 *
 *  Is able to construct vertex declaration elements from itself, calculate vertex
 *  size, etc.
 */
class CVertexFormat {
public:
	/**
	 *  3D vector presence/encoding mode.
	 *		- FLT3_NONE - none present.
	 *		- FLT3_FLOAT3 - plain floats.
	 *		- FLT3_COLOR - color bytes (ARGB), treated as R,G,B,A in 0..1 range.
	 *		- FLT3_DEC3N - DWORD packed with trhee 10 bit signed fixed point components.
	 */
	enum eFloat3Mode { FLT3_NONE = 0, FLT3_FLOAT3, FLT3_COLOR, FLT3_DEC3N, FLT3MASK = 3 };
	/**
	 *  Skinning mode.
	 */
	enum eSkinMode { SKIN_1BONE = 0, SKIN_2BONE, SKIN_3BONE, SKIN_4BONE, SKINMASK = 3 };
	/**
	 *  UV data mode.
	 */
	enum eUVMode { UV_NONE = 0, UV_1D, UV_2D, UV_3D, UVMASK = 3 };

	/** Position component presence flag/bit. Data is float3. */
	enum { POSITION_BIT = 0, POSITION_MASK = 1<<POSITION_BIT };
	/** Normal flags mask/bits. The mode for normals is eFloat3Mode. */
	enum { NORMAL_BITS = 1, NORMAL_MASK = FLT3MASK<<NORMAL_BITS };
	/** Tangent flags mask/bits. The mode is eFloat3Mode. */
	enum { TANGENT_BITS = 3, TANGENT_MASK = FLT3MASK<<TANGENT_BITS };
	/** Binormal flags mask/bits. The mode is eFloat3Mode. */
	enum { BINORM_BITS = 5, BINORM_MASK = FLT3MASK<<BINORM_BITS };
	/** Skin weights count. */
	enum { SKIN_BITS = 7, SKIN_MASK = SKINMASK<<SKIN_BITS };
	/** Skin weights data mode (eFloat3Mode). */
	enum { SKIN_DATA_BITS = 9, SKIN_DATA_MASK = FLT3MASK<<SKIN_DATA_BITS };
	/** UV bits. Data for UV0 is at UV_BITS, for UV1 is at UV_BITS+UV_SHIFT, etc. Up to UV_COUNT. */
	enum { UV_BITS = 11, UV_MASK = UVMASK<<UV_BITS, UV_SHIFT = 2, UV_COUNT = 8 };
	/** Color component flag/bit. Data is COLOR. */
	enum { COLOR_BIT = UV_BITS + UV_COUNT*UV_SHIFT, COLOR_MASK = 1<<COLOR_BIT };


public:
	enum { V_POSITION = POSITION_MASK };
	enum { V_NORMAL = FLT3_FLOAT3<<NORMAL_BITS, V_NORMALCOL = FLT3_COLOR<<NORMAL_BITS, V_NORMALDEC = FLT3_DEC3N<<NORMAL_BITS };
	enum { V_TANGENT = FLT3_FLOAT3<<TANGENT_BITS, V_TANGENTCOL = FLT3_COLOR<<TANGENT_BITS, V_TANGENTDEC = FLT3_DEC3N<<TANGENT_BITS };
	enum { V_BINORM = FLT3_FLOAT3<<BINORM_BITS, V_BINORMCOL = FLT3_COLOR<<BINORM_BITS, V_BINORMDEC = FLT3_DEC3N<<BINORM_BITS };
	enum { V_SKIN_1 = SKIN_1BONE<<SKIN_BITS, V_SKIN_2 = SKIN_2BONE<<SKIN_BITS, V_SKIN_3 = SKIN_3BONE<<SKIN_BITS, V_SKIN_4 = SKIN_4BONE<<SKIN_BITS };
	enum { V_SKIN_WHTNONE = FLT3_NONE<<SKIN_DATA_BITS, V_SKIN_WHT = FLT3_FLOAT3<<SKIN_DATA_BITS, V_SKIN_WHTCOL = FLT3_COLOR<<SKIN_DATA_BITS, V_SKIN_WHTDEC = FLT3_DEC3N<<SKIN_DATA_BITS };
	enum { V_UV0_2D = UV_2D<<(UV_BITS+UV_SHIFT*0) };

public:
	CVertexFormat( DWORD bits = 0 ) : mFormatBits(bits) { }

	//
	// Vertex component flags and modes

	bool		hasPosition() const { return (mFormatBits & POSITION_MASK) ? true : false; }
	eFloat3Mode getNormalMode() const { return eFloat3Mode((mFormatBits&NORMAL_MASK) >> NORMAL_BITS); }
	eFloat3Mode getTangentMode() const { return eFloat3Mode((mFormatBits&TANGENT_MASK) >> TANGENT_BITS); }
	eFloat3Mode getBinormMode() const { return eFloat3Mode((mFormatBits&BINORM_MASK) >> BINORM_BITS); }
	eSkinMode	getSkinMode() const { return eSkinMode((mFormatBits&SKIN_MASK) >> SKIN_BITS); }
	eFloat3Mode getSkinDataMode() const { return eFloat3Mode((mFormatBits&SKIN_DATA_MASK) >> SKIN_DATA_BITS); }
	bool		hasColor() const { return (mFormatBits & COLOR_MASK) ? true : false; }
	eUVMode		getUVMode( int uv = 0 ) const { return eUVMode(( mFormatBits >> (UV_BITS+uv*UV_SHIFT) ) & UVMASK); }

	//
	// The final bits

	DWORD getFormatBits() const { return mFormatBits; }

	//
	// Vertex sizes, component sizes and D3D declaration types

	int calcVertexSize() const;
	static int calcFloat3Size( eFloat3Mode flt3 );
	static int calcFloat3Type( eFloat3Mode flt3 );
	static int calcUVSize( eUVMode uv ) { return uv*4; }
	static int calcUVType( eUVMode uv );
	int calcSkinSize() const;
	int calcSkinDataType() const;

	int calcSkinWeightsOffset() const;
	int calcSkinIndicesOffset() const;
	int calcNormalOffset() const;
	int calcTangentOffset() const;
	int calcBinormOffset() const;

	//
	// D3D vertex declaration building

	/**
	 *  Calculates the number of D3DVERTEXELEMENT9 enties that are needed to
	 *  describe this vertex type.
	 */
	int calcComponentCount() const;

	/**
	 *  Fills in provided D3DVERTEXELEMENT9 array. The array must be of sufficient
	 *  size (at least of calcComponentCount() value).
	 *
	 *  Sets the stream values and usage indices to stream parameter, except for UV
	 *  coordinates - these are numbered from uvIdx.
	 */
	void calcVertexDecl( D3DVERTEXELEMENT9* elems, int stream, int uvIdx ) const;

public:
	bool operator< ( const CVertexFormat& fmt ) const { return mFormatBits < fmt.mFormatBits; }
	bool operator== ( const CVertexFormat& fmt ) const { return mFormatBits == fmt.mFormatBits; }
	bool operator!= ( const CVertexFormat& fmt ) const { return mFormatBits != fmt.mFormatBits; }

private:
	DWORD	mFormatBits;
};


}; // namespace


#endif
