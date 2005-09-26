restored = {
	{ 'AlphaBlendEnable', 'False' },
	{ 'SeparateAlphaBlendEnable', 'False' },
	{ 'AlphaTestEnable', 'False' },
	--{ 'ClipPlaneEnable[n]', 'False' }, TBD
	{ 'ColorWriteEnable', 'Red | Green | Blue | Alpha' },
	{ 'FogEnable', 'False' },
	{ 'PointSpriteEnable', 'False' },
	{ 'StencilEnable', 'False' },
	{ 'ZEnable', 'True' },
	{ 'ZWriteEnable', 'True' },
	{ 'BlendOp', 'Add' },
	{ 'BlendOpAlpha', 'Add' },
	{ 'Clipping', 'True' },
	{ 'CullMode', '<iCull>' },
	{ 'DepthBias', '0' },
	{ 'DitherEnable', 'False' },
	{ 'FillMode', 'Solid' },
	{ 'LastPixel', 'True' },
	{ 'MultiSampleAntiAlias', 'True' },
	{ 'MultiSampleMask', '0xFFFFFFFF' },
	{ 'PatchSegments', '0' },
	{ 'ShadeMode', 'Gouraud' },
	{ 'SlopeScaleDepthBias', '0' },
	{ 'ZFunc', 'Less' },
	--{ 'TexCoordIndex[n]', '' }, TBD
	--{ 'TextureTransformFlags[n]', 'Disable' }, TBD
	--{ 'Wrap[n]', '0' }, TBD
}

required = {
	'VertexShader', 'PixelShader',
}

dependent = {
	{ 'AlphaBlendEnable', 1, { 'SrcBlend', 'DestBlend', }, },
	{ 'SeparateAlphaBlendEnable', 1, { 'SrcBlendAlpha', 'DestBlendAlpha', }, },
	{ 'AlphaTestEnable', 1, { 'AlphaFunc', 'AlphaRef', }, },
	{ 'StencilEnable', 1, { 'StencilFail', 'StencilFunc', 'StencilMask', 'StencilPass', 'StencilWriteMask', 'StencilZFail', }, },
}
