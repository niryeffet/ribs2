TARGET=test_ribs2

ifneq ($(wildcard /usr/include/zlib.h),)
HAVE_ZLIB=1
endif

SRC=test_ribs.c test_kmeans.c test_ds_var_field.c
ifeq ($(HAVE_ZLIB),1)
SRC+=test_zlib.c
endif

CFLAGS+= -I ../../include
LDFLAGS+= -L ../../lib -lribs2
ifeq ($(HAVE_ZLIB),1)
LDFLAGS+= -lribs2_zlib -lz -lm
endif

EXTRA_DEPS=../../lib/libribs2.a
ifeq ($(HAVE_ZLIB),1)
EXTRA_DEPS+=../../lib/libribs2_zlib.a
endif

include ../../make/ribs.mk
