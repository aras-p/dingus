HDR rendering test
by Aras 'NeARAZ' Pranckevicius: http://nesnausk.org/nearaz, nearaz@gmail.com

An attempt to use old 32 bit RGBA rendertarget for HDR, using RGBE8 encoding.
Multisampling should Just Work on all DX9 hardware; read more at
http://nesnausk.org/nearaz/projHDR.html

Usage:
* Run.
* Press arrow keys and A/Z to control the camera; the rest is via UI sliders.
* Press F2 for device selection dialog. Try out anti aliasing.

Requirements:
* Good DX9 video card: pixel shaders 2.0 and floating point textures
  required. Good performance is also a plus!
  
Features:
* HDR rendering: mostly copied from DX SDK sample HDRLighting (tone mapping,
  luminance adaptation, blue shift, bloom).
* Diffuse lighting from environment (9 coeff. SH) and direct sunlight with
  shadow map; using per-vertex ambient occlusion term.
* Model is still a WIP version of St. Anne's Church in Vilnius, Lithuania.
  Modeling is done by Siger, see thread at cgtalk.lt/viewtopic.php?t=2505
  (Lithuanian). The model is not authored with realtime rendering in mind,
  that's why it has approx. 500 thousand vertices at the moment.
* The light probe is courtesy of Paul Debevec: http://www.debevec.org
  You can swap in your own probe in data/tex/HdrEnv.dds - in native DDS
  cubemap or vertical cross HDR format (just rename to .dds).


- Aras, 2005 11 01
