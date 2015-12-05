/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2015 TrueSkills, Inc.

    RIBS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, version 2.1 of the License.

    RIBS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with RIBS.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _DS_LOADER_CODE_GEN__H_
#define _DS_LOADER_CODE_GEN__H_

#include <stdio.h>
#include <vmbuf.h>
#include "ds_types.h"

struct ds_loader_code_gen {
    FILE *file_h;
    FILE *file_c;
    char *db_name;
    char *table_name;
    char *typename;
    struct vmbuf file_list_buf;
};

#define DS_LOADER_CODE_GEN_INITIALIZER {NULL, NULL, NULL, NULL, NULL, VMBUF_INITIALIZER};

int ds_loader_init(struct ds_loader_code_gen *loader , const char *typename, const char *db_name);
int ds_loader_close(struct ds_loader_code_gen *loader);
void ds_loader_table(struct ds_loader_code_gen *loader, const char *table_name);
int ds_loader_field(struct ds_loader_code_gen *loader, const char *name, ds_type_t type);
int ds_loader_idx_o2o(struct ds_loader_code_gen *loader, const char *name, ds_type_t type);
int ds_loader_idx_o2m(struct ds_loader_code_gen *loader, const char *name, ds_type_t type);
int ds_loader_idx_o2o_ht(struct ds_loader_code_gen *loader, const char *name);

#endif
