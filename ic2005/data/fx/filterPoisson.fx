#include "lib/structs.fx"
  
texture		tBase;
sampler		smpBase = sampler_state {
    Texture   = (tBase);
    MipFilter = None;	MinFilter = Linear; MagFilter = Linear;
    AddressU = Clamp; AddressV = Clamp;
};

static const int MAP_SIZE = 256;

SPosTex vsMain11( SPosTex i )
{
    SPosTex o;
	o.pos = i.pos * float4(2,2,1,1);
	o.uv = i.uv + 0.5/MAP_SIZE;
	return o;
}

#define NUM_TAPS 12
half2 filterTaps[NUM_TAPS] = {
	{-0.326212f, -0.405805f},
	{-0.840144f, -0.07358f},
	{-0.695914f,  0.457137f},
	{-0.203345f,  0.620716f},
	{ 0.96234f,  -0.194983f},
	{ 0.473434f, -0.480026f},
	{ 0.519456f,  0.767022f},
	{ 0.185461f, -0.893124f},
	{ 0.507431f,  0.064425f},
	{ 0.89642f,   0.412458f},
	{-0.32194f,  -0.932615f},
	{-0.791559f, -0.597705f}
};


half4 psMain20( float2 tc0 : TEXCOORD0 ) : COLOR
{
	half4 col = tex2D( smpBase, tc0 );

	//return col.r;
	//return col.b;
	//return col.r*0.4+0.6;

	half scaling = col.g + 0.02;
	
	half colorSum = col.b;
	half scale = 12.0/MAP_SIZE;
	
	for( int k = 0; k < NUM_TAPS; ++k ) {
		float2 tapCoord = tc0 + filterTaps[k] * (scale * scaling);
		colorSum += tex2D( smpBase, tapCoord ).b;
	}

	half shadow = colorSum / (NUM_TAPS+1);
	return shadow*0.4+0.6;
}


technique tec20 {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_2_0 psMain20();

		ZEnable = False;
		ZWriteEnable = False;
	}
	pass PLast {
		//Texture[0] = NULL;
		ZEnable = True;
		ZWriteEnable = True;
	}
}
