backdropdir=$(datadir)/xfce4/backdrops

EXTRA_DIST = Blue.list.in Toulouse.list.in 

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
Blue.list.in \
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
Flower.jpg \
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
Horizon.png \
Leaves.xpm \
Linea.xpm \
Marbled.xpm \
Master-Plan.jpg \
Maze.xpm \
Metal-blue.jpg \
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
Purple.xpm \
Reader.xpm \
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
WipeBlue.png \
Wood2.xpm \
Wood-floor.xpm \
Wood.xpm \
Worm.xpm \
xfce-in-the-moon-3.png \
Xfce.jpg \
Xfcelogo.xpm \
Xfcemice.jpg \
XfceStellar.xpm \
Xfce.xpm

# ... and this gets installed
backdrop_DATA =  $(dist_backdrop_DATA) Blue.list Toulouse.list

Blue.list:
	@sed -e "s%XFCE_DIR%$(datadir)/xfce4%" Blue.list.in > Blue.list

Toulouse.list:
	@sed -e "s%XFCE_DIR%$(datadir)/xfce4%" Toulouse.list.in > Toulouse.list

distclean-local:
	@rm -f Blue.list

