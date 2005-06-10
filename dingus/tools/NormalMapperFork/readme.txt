This is a fork of ATI's NormalMapper tool.
Original NormalMapper can be found at http://www.ati.com/developer/tools.html
NormalMapper is (C) ATI Research, Inc.

This fork contains only the NormalMapper portion and doesn't include viewer,
exporters nor other utilities.

Currently the fork is based on NormalMapper 3.2.2 version.

Changes are:

* Completely remade command line arguments.
  Instead of old way
    NormalMapper -ktvr lo.nmf hi.nmf 256 256 100 out.tga
  now you have
    NormalMapper -low lo.nmf -high hi.nmf -x 256 -y 256 -maxd 100 -outn out.tga -corner -tol 0.0000001 -occ 4
  Yes it's much longer but much better to extend. And the old way was running
  out of available letters! :)

* Able to swap Y and Z components in resulting normal map (-swapyz).
  Mostly useful on world space normalmaps, when meshes are exported in
  max-style and your code uses d3d-style coordinate systems.

* Small optimizations and addons
  . Flag leaf nodes in octree; no need for big checks during traversal.
  . Print computation times.
