
ACLOCAL_AMFLAGS  =  -I m4

lib_LTLIBRARIES  =  libeye.la

libeye_la_CFLAGS    =  -Wall -g
libeye_la_CXXFLAGS  =  -Wall -I.. -g

libeye_la_SOURCES  = \
	libeye.cpp matrix.c

libeye_ladir  =  $(includedir)/libeye

libeye_la_HEADERS  =   \
	libeye.hpp matrix.h libeye-template.cpp

nobase_dist_doc_DATA  = \
	README
