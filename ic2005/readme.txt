
in.out.side: the shell
team nesnausk!
imagine cup 2005


--------------------------------------------------------------------------
 ABOUT

"in.out.side: the shell" is a demo program that displays real-time rendered
3D graphics and plays music.

The demo tells a story about breaking the boundaries between individual
person and the outside world. It tells that while our own private "inside"
world may be nice and polished, we're still trapped in it. By breaking the
shell of our own self we can discover the outside world; the world which may
be much more complex, rough and not that nice. However, there we can discover
whole new worlds (among them, other persons) and become free (i.e. not
trapped inside our own shell).

The message we want to tell is:
    Be free to discover
    your self, your time and the world around you.
    Dissolve the boundaries.

The demo is implemented as both a non-interactive realtime rendered short
film and an interactive user experience. The non-interactive version shows
the whole scenario; however, at any time the user can go to "interactive
mode": look around, walk and experiment with "breaking the boundaries"
himself. The interactive part shows off the power of realtime rendering and
physics simulation; such thing won't be possible in a prerendered movie or
animation.

From the technical side, we use many modern computer graphics techniques,
like normal mapping, ambient occlusion, realtime soft shadows, blury
reflections and fullscreen postprocessing effects. Other special parts are
realtime fracture and physics simulation and seamless animation blending.


--------------------------------------------------------------------------
 REQUIREMENTS

* DirectX 9.0c runtime installed.

* Video card that supports pixel shaders 2.0 and has at least 64MB memory
    - Radeon 9800, GeForce 6600 or better recommended, with 128MB
	- On slower cards or 64MB ones, don't go to very high resolutions; use
	  1024x768 or 800x600 or even 640x480 for the very low-end.
	- On GeForce6800 or Radeon X800, use high resolutions and turn on
	  anti-aliasing. It should look good!

  Tested and seems to work on:
    - GeForce 6800, 6600, 6200, FX5700.
	- Radeon X800, 9800, 9700, 9600, 9550, 9500.

* Something like a 2GHz CPU.

* Somewhat recent video card drivers.


--------------------------------------------------------------------------
 TIPS and TRICKS

Yes, there is an interactive mode at the end! Also you can press [space]
at any time to jump directly to it.

Watch the demo several times and note the scroller!

Interactive mode contains options dialog. In fact it can be shown at anytime
during the demo - just press [f1]. Some options take a couple of seconds to
apply, don't worry.


--------------------------------------------------------------------------
 CREDITS

in.out.side: the shell is done by:
  Aras "NeARAZ" Pranckevicius
  Paulius "OneHalf" Liekis
  Antanas "AB" Balvocius
  Raimundas "mookid" Juska


--------------------------------------------------------------------------
 LINKS

* Nesnausk! website					http://www.nesnausk.org


--------------------------------------------------------------------------
 FAQ
 
Q: It says "d3d9.dll not found"!
A: Install DirectX9.0c runtime.

Q: It says "No compatible D3D devices found. Pixel shaders 2.0 required"!
A: Yes, a video card that supports pixel shaders 2.0 is required. Everything
   from Radeon 9500, GeForce FX5200 and up should work.

Q: It has weird noisy things on the screen, everything is blurred, it
   displays garbage!
A: We suspect a video card driver bug. Try updating your drivers.


--------------------------------------------------------------------------
 MISC

The demo uses these external libraries:
. (modified) ODE for collision detection and physics   http://www.ode.org
. BASS to play music                                   http://www.un4seen.com
. LUA scripting language for some configuration files  http://www.lua.org
