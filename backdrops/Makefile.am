backdropdir=$(datadir)/xfce4/backdrops

EXTRA_DIST = Blue.list.in

# this and the above get included in the tarball ...
dist_backdrop_DATA = \
A-Little-Exercise-3.jpg \
A-Little-Exercise.jpg \
Anneaux.xpm \
Arena.xpm \
Aurora.xpm \
BlackStripes.xpm \
Bluedrops.xpm \
Bluefalls.xpm \
BlueSatin.xpm \
BlueSea.xpm \
BlueStripes.xpm \
BlueStucco.xpm \
Boucles.xpm \
Brownie.xpm \
Candies.xpm \
Cds.xpm \
Circle.xpm \
Cloth.xpm \
Clouds.xpm \
Cocoa.xpm \
Compute.xpm \
Concrete.xpm \
Darkblue2.xpm \
Default.xpm \
Desert2.xpm \
Dry.xpm \
Eyes.xpm \
Fire.xpm \
Flames.xpm \
Flow.xpm \
Funkyblue.xpm \
Gallets.xpm \
Glasses.xpm \
Gold.xpm \
Gradient.xpm \
Greenslate.xpm \
GreenStripes.xpm \
GreenStucco.xpm \
Green.xpm \
GreyArena.xpm \
Greydrops.xpm \
Greysea.xpm \
Grey.xpm \
Hardblue.xpm \
Honey.xpm \
Leaves.xpm \
Marbled.xpm \
Master-Plan.jpg \
Maze.xpm \
Nebula.xpm \
Notes.xpm \
Overdriven.jpg \
Pencils.xpm \
Plasma.xpm \
Pled.xpm \
Pool.xpm \
Psy2.xpm \
Psy3.xpm \
Psy.xpm \
PurpleStripes.xpm \
Rotin.xpm \
Rough.xpm \
Sandy-ridges.xpm \
Satin2.xpm \
Satin3.xpm \
Satin.xpm \
Sea.xpm \
Sky2.xpm \
Sky3.jpg \
Sky.xpm \
Slate.xpm \
Softblue2.xpm \
Softblue.xpm \
Soft.xpm \
Solid.xpm \
Sol.xpm \
Space2.xpm \
Space.xpm \
Storm.xpm \
Sunny.xpm \
Teal.xpm \
The-Good-Times.jpg \
TheLake.jpg \
Tiles2.xpm \
Tiles3.xpm \
Tiles.xpm \
Time-For-Expansion.jpg \
Time.xpm \
Toulouse-1.jpg \
Toulouse-2.jpg \
Toulouse-3.jpg \
Toulouse-4.jpg \
Ufo.xpm \
Wall1.xpm \
Wall2.xpm \
Wall3.xpm \
WaterPurple.xpm \
Wax.xpm \
Weave-red.xpm \
Weird.xpm \
Whirl.xpm \
WhiteStripes.xpm \
Wood2.xpm \
Wood-floor.xpm \
Wood.xpm \
Worm.xpm \
Xfce.jpg \
Xfcelogo.xpm \
Xfcemice.jpg \
Xfce.xpm

# ... and this gets installed
backdrop_DATA =  $(dist_backdrop_DATA) Blue.list

Blue.list:
	@sed -e "s%XFCE_DIR%$(datadir)/xfce4%" Blue.list.in > Blue.list

distclean-local:
	@rm -f Blue.list

