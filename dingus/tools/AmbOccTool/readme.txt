Ambient occlusion calculation on the GPU tool

Usage: AmbOccTool <options>
	-m <mesh>	Input mesh file
	-n <nmap>	Object space normal map of the mesh (optional)
	-x <int>	Output texture width
	-y <int>	Output texture height
	-s			If given - swap Y/Z in the input normal map
	-t			If given - output TGA instead of DDS
	-u			If given - compute UV parametrization
	
Input mesh must contain unique UV parametrization in the first texture
channel, or -u option must be given (careful on big meshes).

Supplying input normal map or output width/height are required. If input
normal map is given, output is normal map (probably Y/Z swapped) with AO
placed in alpha channel. If no input normal map is given, output is
grayscale AO with opaque alpha channel.