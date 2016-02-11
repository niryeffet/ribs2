/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2016 TrueSkills, Inc.

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
#include <stdint.h>
#include <stdio.h>
#include "ds_code_gen_index_container.h"
#include "ds_code_gen_common.h"

static FILE *index_container_file;

void struct_index_container_o2o(char *type) {
    write_code(index_container_file, "\n\nstruct index_container_o2o_%s {\n", type);
    write_code(index_container_file, "    struct file_mapper fm;\n");
    write_code(index_container_file, "    struct index_key_rec_o2o_%s *data;\n", type);
    write_code(index_container_file, "    struct index_header_o2o *header;\n");
    write_code(index_container_file, "};");
}

void index_container_o2o_bsearch(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2o_bsearch_%s(struct index_key_rec_o2o_%s *a, size_t n, %s key, uint32_t *loc) {\n", type, type, type);
    write_code(index_container_file, "    uint32_t l = 0, h = n;\n");
    write_code(index_container_file, "    while (l < h) {\n");
    write_code(index_container_file, "        uint32_t m = (l + h) >> 1;\n");
    write_code(index_container_file, "        if (a[m].key < key) {\n");
    write_code(index_container_file, "            l = m;\n");
    write_code(index_container_file, "            ++l;\n");
    write_code(index_container_file, "        } else {\n");
    write_code(index_container_file, "            h = m;\n");
    write_code(index_container_file, "        }\n");
    write_code(index_container_file, "    }\n");
    write_code(index_container_file, "    if (l < n && a[l].key == key)\n");
    write_code(index_container_file, "        return *loc = a[l].row, 0;\n");
    write_code(index_container_file, "    return -1;\n");
    write_code(index_container_file, "};");
}

void index_container_o2o_init(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2o_init_%s(struct index_container_o2o_%s *ic, const char *filename) {\n", type, type);
    write_code(index_container_file, "    if (0 > file_mapper_init(&ic->fm, filename))\n");
    write_code(index_container_file, "        return -1;\n");
    write_code(index_container_file, "    if (file_mapper_size(&ic->fm) < sizeof(struct index_header_o2o))\n");
    write_code(index_container_file, "        return LOGGER_ERROR(\"invalid file size: %%s\", filename), file_mapper_free(&ic->fm), -1;\n");
    write_code(index_container_file, "    ic->header = file_mapper_data(&ic->fm);\n");
    write_code(index_container_file, "    ic->data = file_mapper_data(&ic->fm) + sizeof(struct index_header_o2o);\n");
    write_code(index_container_file, "    if (0 != memcmp(ic->header->signature, IDX_O2O_SIGNATURE, sizeof(ic->header->signature)))\n");
    write_code(index_container_file, "        return LOGGER_ERROR(\"corrupted file: %%s\", filename), file_mapper_free(&ic->fm), -1;\n");
    write_code(index_container_file, "    return 0;\n");
    write_code(index_container_file, "};");
}

void index_container_o2o_init2(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2o_init2_%s(struct index_container_o2o_%s *ic, const char *base_path, const char *db, const char *table, const char *field) {\n", type, type);
    write_code(index_container_file, "    char filename[PATH_MAX];\n");
    write_code(index_container_file, "    if (PATH_MAX <= snprintf(filename, PATH_MAX, \"%%s/%%s/%%s/%%s.idx\", base_path, db, table, field))\n");
    write_code(index_container_file, "        return LOGGER_ERROR(\"filename too long\"), -1;\n");
    write_code(index_container_file, "    return file_mapper_init(&ic->fm, filename);\n");
    write_code(index_container_file, "};");
}

void index_container_o2o_lookup(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2o_lookup_%s(struct index_container_o2o_%s *ic, %s key, uint32_t *loc) {\n", type, type, type);
    write_code(index_container_file, "    return index_container_o2o_bsearch_%s(ic->data, ic->header->num_keys, key, loc);\n", type);
    write_code(index_container_file, "};");
}

void index_container_o2o_exist(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2o_exist_%s(struct index_container_o2o_%s *ic, %s key) {\n", type, type, type);
    write_code(index_container_file, "    uint32_t loc;\n");
    write_code(index_container_file, "    return index_container_o2o_bsearch_%s(ic->data, ic->header->num_keys, key, &loc);\n", type);
    write_code(index_container_file, "};");
}

void index_container_o2o_free(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2o_free_%s(struct index_container_o2o_%s *ic) {\n", type, type);
    write_code(index_container_file, "    return file_mapper_free(&ic->fm);\n");
    write_code(index_container_file, "};");

}

void struct_index_container_o2m(char *type) {
    write_code(index_container_file, "struct index_container_o2m_%s {\n", type);
    write_code(index_container_file, "    struct file_mapper fm;\n");
    write_code(index_container_file, "    struct index_header_o2m *header;\n");
    write_code(index_container_file, "    uint32_t *vect;\n");
    write_code(index_container_file, "    struct index_key_rec_o2m_%s *keys;\n", type);
    write_code(index_container_file, "};");

}

void index_container_o2m_init(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2m_init_%s(struct index_container_o2m_%s *ic, const char *filename) {\n", type, type);
    write_code(index_container_file, "    if (0 > file_mapper_init(&ic->fm, filename))\n");
    write_code(index_container_file, "        return -1;\n");
    write_code(index_container_file, "    if (file_mapper_size(&ic->fm) < sizeof(struct index_header_o2m))\n");
    write_code(index_container_file, "        return LOGGER_ERROR(\"invalid file size: %%s\", filename), file_mapper_free(&ic->fm), -1;\n");
    write_code(index_container_file, "    ic->header = file_mapper_data(&ic->fm);\n");
    write_code(index_container_file, "    if (0 != memcmp(ic->header->signature, IDX_O2M_SIGNATURE, sizeof(ic->header->signature)))\n");
    write_code(index_container_file, "        return LOGGER_ERROR(\"corrupted file: %%s\", filename), file_mapper_free(&ic->fm), -1;\n");
    write_code(index_container_file, "    ic->vect = file_mapper_data(&ic->fm) + sizeof(struct index_header_o2m);\n");
    write_code(index_container_file, "    ic->keys = file_mapper_data(&ic->fm) + ic->header->keys_ofs;\n");
    write_code(index_container_file, "    return 0;\n");
    write_code(index_container_file, "};");
}

void index_container_o2m_bsearch(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2m_bsearch_%s(struct index_key_rec_o2m_%s *a, size_t n, %s key, uint32_t *loc) {\n", type, type, type);
    write_code(index_container_file, "    uint32_t l = 0, h = n;\n");
    write_code(index_container_file, "    while (l < h) {\n");
    write_code(index_container_file, "        uint32_t m = (l + h) >> 1;\n");
    write_code(index_container_file, "        if (a[m].key < key) {\n");
    write_code(index_container_file, "            l = m;\n");
    write_code(index_container_file, "            ++l;\n");
    write_code(index_container_file, "        } else {\n");
    write_code(index_container_file, "            h = m;\n");
    write_code(index_container_file, "        }\n");
    write_code(index_container_file, "    }\n");
    write_code(index_container_file, "    if (l < n && a[l].key == key)\n");
    write_code(index_container_file, "        return *loc = l, 0;\n");
    write_code(index_container_file, "    return -1;\n");
    write_code(index_container_file, "};");
}

void index_container_o2m_lookup(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2m_lookup_%s(struct index_container_o2m_%s *ic, %s key, uint32_t **vect, uint32_t *size) {\n", type, type, type);
    write_code(index_container_file, "    uint32_t loc;\n");
    write_code(index_container_file, "    if (0 > index_container_o2m_bsearch_%s(ic->keys, ic->header->num_keys, key, &loc))\n", type);
    write_code(index_container_file, "        return -1;\n");
    write_code(index_container_file, "    struct index_key_rec_o2m_%s *key_rec = ic->keys + loc;\n", type);
    write_code(index_container_file, "    *vect = ic->vect + key_rec->vect;\n");
    write_code(index_container_file, "    *size = key_rec->size;\n");
    write_code(index_container_file, "    return 0;\n");
    write_code(index_container_file, "};");
}

void index_container_o2m_exist(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2m_exist_%s(struct index_container_o2m_%s *ic, %s key) {\n", type, type, type);
    write_code(index_container_file, "    uint32_t loc;\n");
    write_code(index_container_file, "    return index_container_o2m_bsearch_%s(ic->keys, ic->header->num_keys, key, &loc);\n", type);
    write_code(index_container_file, "};");
}

void index_container_o2m_free(char *type) {
    write_code(index_container_file, "\n\nstatic inline int index_container_o2m_free_%s(struct index_container_o2m_%s *ic) {\n", type, type);
    write_code(index_container_file, "    return file_mapper_free(&ic->fm);\n");
    write_code(index_container_file, "};");
}

void ds_code_gen_index_container(const char *filename) {
    if (!(index_container_file = fopen(filename, "w")))
        die_perror("fopen");

    write_generated_file_comment(index_container_file, __FILE__);
    write_code(index_container_file, "#include <limits.h>");
    uint32_t i = 0;
    for (; i < sizeof(ds_types) / sizeof(ds_types[0]); i++) {
        struct_index_container_o2o(ds_types[i]);
        index_container_o2o_bsearch(ds_types[i]);
        index_container_o2o_init(ds_types[i]);
        index_container_o2o_init2(ds_types[i]);
        index_container_o2o_lookup(ds_types[i]);
        index_container_o2o_exist(ds_types[i]);
        index_container_o2o_free(ds_types[i]);
        struct_index_container_o2m(ds_types[i]);
        index_container_o2m_init(ds_types[i]);
        index_container_o2m_bsearch(ds_types[i]);
        index_container_o2m_lookup(ds_types[i]);
        index_container_o2m_exist(ds_types[i]);
        index_container_o2m_free(ds_types[i]);
    }

    if (0 != fclose(index_container_file))
        die_perror("fclose");
}
