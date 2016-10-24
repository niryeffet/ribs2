TARGET=ds_code_gen
SRC=ds_code_gen.c ds_code_gen_index_gen.c ds_code_gen_index_container.c
CFLAGS+= -I ../../include
GEN_OUTPUT=../../include/index_gen.h
BIN_DIR=../../bin

buildall: all $(GEN_OUTPUT)

include ../../make/ribs.mk

$(GEN_OUTPUT): $(TARGET_FILE)
	$(TARGET_FILE) ../../include index_gen.h index_container.h
