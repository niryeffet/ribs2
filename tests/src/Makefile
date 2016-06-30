TARGET=test_ribs2

SRC=test_ribs.c test_kmeans.c test_ds_var_field.c test_zlib.c

CFLAGS+= -I ../../include
LDFLAGS+= -L ../../lib -lribs2 -lribs2_zlib -lz -lm

EXTRA_DEPS=../../lib/libribs2.a ../../lib/libribs2_zlib.a

include ../../make/ribs.mk

