# This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
# RIBS is an infrastructure for building great SaaS applications (but not
# limited to).

# Copyright (C) 2012 Adap.tv, Inc.

# RIBS is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 2.1 of the License.

# RIBS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with RIBS.  If not, see <http://www.gnu.org/licenses/>.

ifndef OPTFLAGS
OPTFLAGS=-O3
endif
ifndef OBJ_SUB_DIR
OBJ_DIR=../obj
else
OBJ_DIR=../obj/$(OBJ_SUB_DIR)
endif

ifndef BIN_DIR
BIN_DIR=../bin
endif

ifneq ($(wildcard /usr/include/zlib.h),)
CPPFLAGS+=-DHAVE_ZLIB
endif

ifeq ($(RIBS2_SSL),1)
CPPFLAGS+=-DRIBS2_SSL
endif

LDFLAGS+=-L../lib
CFLAGS+=$(OPTFLAGS) -ggdb3 -W -Wall -Werror -Wextra
GCCVER_GTE_4_7=$(shell expr `gcc -dumpversion` \>= 4.7)
ifeq ($(GCCVER_GTE_4_7),1)
CFLAGS+=-ftrack-macro-expansion=2
endif

RIBIFY_SYMS+=write read socket connect fcntl recv recvfrom recvmsg send sendto sendmsg readv writev pipe pipe2 nanosleep usleep sleep sendfile close
ifeq ($(RIBIFY_MALLOC),1)
RIBIFY_SYMS+=malloc calloc realloc free strdup wcsdup malloc_usable_size
endif

ifdef UGLY_GETADDRINFO_WORKAROUND
LDFLAGS+=-lanl
RIBIFY_SYMS+=getaddrinfo
CPPFLAGS+=-DUGLY_GETADDRINFO_WORKAROUND
endif

RIBIFYFLAGS+=$(subst --redefine-sym_,--redefine-sym ,$(join $(RIBIFY_SYMS:%=--redefine-sym_%=),$(RIBIFY_SYMS:%=_ribified_%)))

OBJ=$(SRC:%.c=$(OBJ_DIR)/%.o) $(ASM:%.S=$(OBJ_DIR)/%.o)
DEP=$(SRC:%.c=$(OBJ_DIR)/%.d)

DIRS=$(OBJ_DIR)/.dir $(BIN_DIR)/.dir ../lib/.dir
RIBIFY_DIR=../ribified/.dir
ALL_DIRS=$(DIRS) $(RIBIFY_DIR)
ALL_OUTPUT_FILES=$(patsubst %,$(OBJ_DIR)/%,*.o *.d) ../ribified/*

ifeq ($(TARGET:%.a=%).a,$(TARGET))
LIB_OBJ:=$(OBJ)
TARGET_FILE=../lib/lib$(TARGET)
else
TARGET_FILE=$(BIN_DIR)/$(TARGET)
endif

DS_TARGETS_C=$(filter ds_loader_%.c, $(SRC))
DS_TARGETS=$(DS_TARGETS_C) $(DS_TARGETS_C:%.c=%.h)
OBJ_DS=$(DS_TARGETS_C:ds_loader_%.c=$(OBJ_DIR)/%_ds.o)
DEP_DS=$(DS_TARGETS_C:ds_loader_%.c=$(OBJ_DIR)/%_ds.d)

ALL_OUTPUT_FILES+=$(DS_TARGETS) $(TARGET_FILE) $(DS_TARGETS_C:ds_loader_%.c=$(BIN_DIR)/%_ds)

all: $(TARGET_FILE)

$(ALL_DIRS):
	@echo "  (MKDIR)  -p $(@:%/.dir=%)"
	@-mkdir -p $(@:%/.dir=%)
	@touch $@

ds_loader_%.c ds_loader_%.h: $(BIN_DIR)/%_ds
	@echo "  (EXEC)     $*_ds"
	@RIBS_DS_NAME=ds_loader_$* $(BIN_DIR)/$*_ds

$(BIN_DIR)/%_ds: $(OBJ_DS)
	@echo "  (LD)     $(@:$(BIN_DIR)/%=%)  [ -o $@ $(OBJ_DIR)/$*_ds.o $(LDFLAGS) ]"
	@$(CC) -o $@ $(OBJ_DIR)/$*_ds.o $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/%.d
	@echo "  (C)      $*.c  [ $(CPPFLAGS) -c $(CFLAGS) $*.c -o $(OBJ_DIR)/$*.o ]"
	@$(CC) $(CPPFLAGS) -c $(CFLAGS) $*.c -o $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%.o: %.S
	@echo "  (ASM)    $*.S  [ $(CPPFLAGS) -c $(CFLAGS) $*.S -o $(OBJ_DIR)/$*.o ]"
	@$(CC) $(CPPFLAGS) -c $(CFLAGS) $*.S -o $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%_ds.d: %_ds.c
	@echo "  (DEP)    $*_ds.c"
	@$(CC) -MM $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $*_ds.c | sed -e 's|.*:|$(OBJ_DIR)/$*_ds.o:|' > $@

$(OBJ_DIR)/%.d: %.c
	@echo "  (DEP)    $*.c"
	@$(CC) -MM $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $*.c | sed -e 's|.*:|$(OBJ_DIR)/$*.o:|' > $@

$(OBJ) $(OBJ_DS): $(DIRS)

$(DEP) $(DEP_DS): $(DIRS)
$(DEP): $(DS_TARGETS)

../lib/%: $(LIB_OBJ)
	@echo "  (AR)     $(@:../lib/%=%)  [ rcs $@ $^ ]"
	@$(AR) rcs $@ $^

.PRECIOUS: $(RIBIFY:%=../ribified/%)

../ribified/%: $(RIBIFY_DIR)
	@echo "  (RIBIFY) $(@:../ribified/%=%) [ $@ $(RIBIFYFLAGS) ]"
	@objcopy $(shell find $(RIBIFY_LIB_PATH) /usr/lib -name $(@:../ribified/%=%) 2>/dev/null) $@ $(RIBIFYFLAGS)

$(BIN_DIR)/$(TARGET): $(OBJ) $(RIBIFY:%=../ribified/%) $(EXTRA_DEPS)
	@echo "  (LD)     $(@:$(BIN_DIR)/%=%)  [ -o $@ $(OBJ) $(LDFLAGS) ]"
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

$(ALL_OUTPUT_FILES:%=%.__clean__):
	@echo "  (RM)     $(@:%.__clean__=%)"
	@-$(RM) $(@:%.__clean__=%)

clean: $(ALL_OUTPUT_FILES:%=%.__clean__)

etags:
	@echo "  (ETAGS)"
	@find . -name "*.[ch]" | cut -c 3- | xargs etags -I

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif
