# Process this file with automake to produce Makefile.in

AM_CXXFLAGS=@NOVA_CFLAGS@ @CFITSIO_CFLAGS@ @LIBPG_CFLAGS@ @LIBXML_CFLAGS@ @MAGIC_CFLAGS@ -I../../include -I../

noinst_HEADERS = rts2prop.h rts2targetplanet.h augerset.h

EXTRA_DIST = sqlerror.ec devicedb.ec target.ec sub_targets.ec appdb.ec sqlcolumn.ec observation.ec \
	observationset.ec taruser.ec rts2count.ec imageset.ec targetset.ec plan.ec planset.ec rts2prop.ec \
	camlist.ec target_auger.ec messagedb.ec targetgrb.ec \
	user.ec userset.ec account.ec accountset.ec recvals.ec records.ec recordsavg.ec \
	augerset.ec labels.ec labellist.ec queues.ec

CLEANFILES = sqlerror.cpp devicedb.cpp target.cpp sub_targets.cpp appdb.cpp sqlcolumn.cpp observation.cpp \
	observationset.cpp taruser.cpp rts2count.cpp imageset.cpp targetset.cpp plan.cpp planset.cpp rts2prop.cpp \
	camlist.cpp target_auger.cpp messagedb.cpp targetgrb.cpp \
	user.cpp userset.cpp account.cpp accountset.cpp recvals.cpp records.cpp recordsavg.cpp \
	augerset.cpp labels.cpp labellist.cpp queues.cpp

if PGSQL

lib_LTLIBRARIES = librts2db.la

nodist_librts2db_la_SOURCES = sqlerror.cpp devicedb.cpp target.cpp sub_targets.cpp appdb.cpp sqlcolumn.cpp observation.cpp \
	observationset.cpp taruser.cpp rts2count.cpp imageset.cpp targetset.cpp plan.cpp planset.cpp \
	rts2prop.cpp camlist.cpp target_auger.cpp messagedb.cpp rts2targetplanet.cpp targetgrb.cpp \
	targetell.cpp tletarget.cpp user.cpp userset.cpp account.cpp accountset.cpp recvals.cpp records.cpp recordsavg.cpp \
	augerset.cpp labels.cpp labellist.cpp queues.cpp targetres.cpp simbadtargetdb.cpp

librts2db_la_SOURCES = mpectarget.cpp imagesetstat.cpp constraints.cpp
librts2db_la_LIBADD = ../rts2fits/librts2imagedb.la ../rts2/librts2.la ../pluto/libpluto.la ../xmlrpc++/librts2xmlrpc.la \
	@LIBPG_LIBS@ @LIBXML_LIBS@ @CFITSIO_LIBS@ @MAGIC_LIBS@ @LIB_ECPG@ @LIB_CRYPT@

.ec.cpp:
	@ECPG@ -o $@ $^

else

EXTRA_DIST += mpectarget.cpp imagesetstat.cpp constraints.cpp

endif
