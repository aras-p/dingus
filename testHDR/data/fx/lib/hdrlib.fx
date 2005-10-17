// HDR utilities

#ifndef __HDR_LIB_FX
#define __HDR_LIB_FX


// --------------------------------------------------------------------------

/**
 * RGBE8 Encoding/Decoding
 * The RGBE8 format stores a mantissa per color channel and a shared exponent
 * stored in alpha. Since the exponent is shared, it's computed based on the
 * highest intensity color component. The resulting color is RGB * 2^Alpha,
 * which scales the data across a logarithmic scale.
 */
float4 EncodeRGBE8( in float3 rgb )
{
	float4 vEncoded;

    // Determine the largest color component
	float maxComponent = max( max(rgb.r, rgb.g), rgb.b );
	
	// Round to the nearest integer exponent
	float fExp = ceil( log2(maxComponent) );

    // Divide the components by the shared exponent
	vEncoded.rgb = rgb / exp2(fExp);
	
	// Store the shared exponent in the alpha channel
	vEncoded.a = (fExp + 128) / 255;

	return vEncoded;
}

float3 DecodeRGBE8( in float4 rgbe )
{
	float3 vDecoded;

    // Retrieve the shared exponent
	float fExp = rgbe.a * 255 - 128;
	
	// Multiply through the color components
	vDecoded = rgbe.rgb * exp2(fExp);
	
	return vDecoded;
}


// --------------------------------------------------------------------------

/**
 * RE8 Encoding/Decoding
 * The RE8 encoding is simply a single channel version of RGBE8, useful for
 * storing non-color floating point data (such as calculated scene luminance)
 */
float4 EncodeRE8( in float f )
{
    float4 vEncoded = float4( 0, 0, 0, 0 );
    
    // Round to the nearest integer exponent
    float fExp = ceil( log2(f) );
    
    // Divide by the exponent
    vEncoded.r = f / exp2(fExp);
    
    // Store the exponent
    vEncoded.a = (fExp + 128) / 255;
    
    return vEncoded;
}

float DecodeRE8( in float4 rgbe )
{
    float fDecoded;

    // Retrieve the shared exponent
	float fExp = rgbe.a * 255 - 128;
	
	// Multiply through the color components
	fDecoded = rgbe.r * exp2(fExp);

	return fDecoded;  
}


#endif
