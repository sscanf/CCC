#
# Program: Map-27
#

.c.obj:
	qcl -c  -W1 -Ze -AM $*.c

trunking.obj : trunking.c

map_tim.obj : map_tim.c

map_sys.obj : map_sys.c

map_rs.obj : map_rs.c

map_dl.obj : map_dl.c

map_mod.obj : map_mod.c

map_modl.obj : map_modl.c

map_net.obj : map_net.c

Map-27.exe : trunking.obj map_tim.obj map_sys.obj map_rs.obj map_dl.obj  \
      map_mod.obj map_modl.obj map_net.obj

	del Map-27.lnk
	echo trunking.obj+ >>Map-27.lnk
	echo map_tim.obj+ >>Map-27.lnk
	echo map_sys.obj+ >>Map-27.lnk
	echo map_rs.obj+ >>Map-27.lnk
	echo map_dl.obj+ >>Map-27.lnk
	echo map_mod.obj+ >>Map-27.lnk
	echo map_modl.obj+ >>Map-27.lnk
	echo map_net.obj  >>Map-27.lnk
	echo Map-27.exe >>Map-27.lnk
	echo Map-27.map >>Map-27.lnk
	link @Map-27.lnk /NOI $(LDFLAGS);
