-- If these states are modified, they are restored to the given default values
restored = {
	-- render states
	{ 'AlphaBlendEnable', 'False' },
	{ 'SeparateAlphaBlendEnable', 'False' },
	{ 'AlphaTestEnable', 'False' },
	{ 'ClipPlaneEnable', '0' },
	{ 'ColorWriteEnable', 'Red | Green | Blue | Alpha' },
	{ 'FogEnable', 'False' },
	{ 'PointSpriteEnable', 'False' },
	{ 'StencilEnable', 'False' },
	{ 'ZEnable', 'True' },
	{ 'ZWriteEnable', 'True' },
	{ 'BlendOp', 'Add' },
	{ 'BlendOpAlpha', 'Add' },
	{ 'Clipping', 'True' },
	{ 'CullMode', '(iCull)' }, -- standard cull mode is iCull shared variable
	{ 'DepthBias', '0' },
	{ 'DitherEnable', 'False' },
	{ 'FillMode', '(iFill)' }, -- standard fill mode is iFill shared variable
	{ 'LastPixel', 'True' },
	{ 'MultiSampleAntiAlias', 'True' },
	{ 'MultiSampleMask', '0xFFFFFFFF' },
	{ 'PatchSegments', '0' },
	{ 'ShadeMode', 'Gouraud' },
	{ 'SlopeScaleDepthBias', '0' },
	{ 'SRGBWriteEnable', 'False' },
	{ 'ZFunc', 'Less' },
	{ 'Wrap0', '0' },
	{ 'Wrap1', '0' },
	{ 'Wrap2', '0' },
	{ 'Wrap3', '0' },
	{ 'Wrap4', '0' },
	{ 'Wrap5', '0' },
	{ 'Wrap6', '0' },
	{ 'Wrap7', '0' },
	{ 'Wrap8', '0' },
	{ 'Wrap9', '0' },
	{ 'Wrap10', '0' },
	{ 'Wrap11', '0' },
	{ 'Wrap12', '0' },
	{ 'Wrap13', '0' },
	{ 'Wrap14', '0' },
	{ 'Wrap15', '0' },
	-- exotic texture stage states
	{ 'TexCoordIndex', '@stage@' }, -- value is the stage index
	{ 'TextureTransformFlags', 'Disable' },
	-- exotic sampler states
	{ 'MipMapLodBias', '0' },
	{ 'MaxMipLevel', '0' },
	{ 'SRGBTexture', '0' },
}

-- These states are required in first pass of each effect
required = {
	'VertexShader', 'PixelShader',
}

-- If the given master state is set to the given value, all listed
-- dependent states must also be set in the same pass.
dependent = {
	{ 'AlphaBlendEnable', 1, { 'SrcBlend', 'DestBlend', }, },
	{ 'SeparateAlphaBlendEnable', 1, { 'SrcBlendAlpha', 'DestBlendAlpha', }, },
	{ 'AlphaTestEnable', 1, { 'AlphaFunc', 'AlphaRef', }, },
	{ 'StencilEnable', 1, { 'StencilFail', 'StencilFunc', 'StencilMask', 'StencilPass', 'StencilWriteMask', 'StencilZFail', }, },
}
