Ambient occlusion calculation on the GPU tool

Usage: AmbOccTool <options>
	-m <mesh>	Input mesh file
	-n <nmap>	Object space normal map of the mesh (optional)
	-x <int>	Output texture width
	-y <int>	Output texture height
	-s			If given - swap Y/Z in the input normal map
	-t			If given - output TGA instead of DDS
	-v			Compute AO per vertex
	
Input mesh must contain unique UV parametrization in the first texture
channel, or "per vertex" option must be used.

Supplying input normal map, output width/height or per-vertex flag is
required.

* If input normal map is given: output is normal map (probably Y/Z swapped)
  with AO placed in alpha channel.
* If no input normal map is given: output is grayscale AO with opaque alpha
  channel.
* If "per vertex" flag is given: output is binary file, with 1 byte per input
  mesh vertex; bytes encode AO.