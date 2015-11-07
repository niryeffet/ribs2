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
#include "ds_code_gen_index_gen.h"
#include "ds_code_gen_common.h"

static FILE *index_gen_file;

void struct_fw_index(char *type) {
    write_code(index_gen_file, "\n\nstruct index_gen_fw_index_%s {\n", type);
    write_code(index_gen_file, "    %s key;\n", type);
    write_code(index_gen_file, "    uint32_t row_loc;\n");
    write_code(index_gen_file, "};");
}

void struct_index_key_rec_o2o(char *type) {
    write_code(index_gen_file, "\n\nstruct index_key_rec_o2o_%s {\n", type);
    write_code(index_gen_file, "    %s key;\n", type);
    write_code(index_gen_file, "    uint32_t row;\n");
    write_code(index_gen_file, "};");
}

void struct_index_key_rec_o2m(char *type) {
    write_code(index_gen_file, "\n\nstruct index_key_rec_o2m_%s {\n", type);
    write_code(index_gen_file, "    %s key;\n", type);
    write_code(index_gen_file, "    uint32_t size;\n");
    write_code(index_gen_file, "    uint32_t vect;\n");
    write_code(index_gen_file, "};");
}

void index_gen_fw_compar(char *type){
    write_code(index_gen_file, "\n\nstatic inline int index_gen_fw_compar_%s(const void *a, const void *b) {\n", type);
    write_code(index_gen_file, "    struct index_gen_fw_index_%s *aa = (struct index_gen_fw_index_%s *)a;\n", type, type);
    write_code(index_gen_file, "    struct index_gen_fw_index_%s *bb = (struct index_gen_fw_index_%s *)b;\n", type, type);
    write_code(index_gen_file, "    return aa->key < bb->key ? -1 : (bb->key < aa->key ? 1 : (aa->row_loc < bb->row_loc ? -1 : (bb->row_loc < aa->row_loc ? 1 : 0))) ;\n");
    write_code(index_gen_file, "}");
}

void index_gen_generate_mem_o2o(char *type) {
    write_code(index_gen_file, "\n\nstatic inline int index_gen_generate_mem_o2o_%s(struct index_gen_fw_index_%s *fw_index, size_t n, const char *filename) {\n", type, type);
    write_code(index_gen_file, "    qsort(fw_index, n, sizeof(struct index_gen_fw_index_%s), index_gen_fw_compar_%s);\n", type, type);
    write_code(index_gen_file, "    struct index_gen_fw_index_%s *fw = fw_index, *fw_end = fw + n;\n", type);
    write_code(index_gen_file, "    struct file_writer fw_idx = FILE_WRITER_INITIALIZER;\n");
    write_code(index_gen_file, "    if (0 > file_writer_init(&fw_idx, filename))\n");
    write_code(index_gen_file, "        return -1;\n");
    write_code(index_gen_file, "    struct index_header_o2o header;\n");
    write_code(index_gen_file, "    memset(&header, 0, sizeof(header));\n");
    write_code(index_gen_file, "    file_writer_write(&fw_idx, &header, sizeof(header));\n");
    write_code(index_gen_file, "    uint32_t num_keys = 0;\n");
    write_code(index_gen_file, "    for (; fw != fw_end; ++num_keys) {\n");
    write_code(index_gen_file, "        struct index_key_rec_o2o_%s output_rec = { fw->key, fw->row_loc };\n", type);
    write_code(index_gen_file, "        %s key = fw->key;\n", type);
    write_code(index_gen_file, "        for (++fw; fw != fw_end && key == fw->key; ++fw);\n");
    write_code(index_gen_file, "        file_writer_write(&fw_idx, &output_rec, sizeof(output_rec));\n");
    write_code(index_gen_file, "    }\n");
    write_code(index_gen_file, "    size_t ofs = file_writer_wlocpos(&fw_idx);\n");
    write_code(index_gen_file, "    file_writer_lseek(&fw_idx, 0, SEEK_SET);\n");
    write_code(index_gen_file, "    header = (struct index_header_o2o){ IDX_O2O_SIGNATURE, num_keys };\n");
    write_code(index_gen_file, "    file_writer_write(&fw_idx, &header, sizeof(header));\n");
    write_code(index_gen_file, "    file_writer_lseek(&fw_idx, ofs, SEEK_SET);\n");
    write_code(index_gen_file, "    file_writer_close(&fw_idx);\n");
    write_code(index_gen_file, "    return 0;\n");
    write_code(index_gen_file, "}");
}

void index_gen_generate_mem_o2m(char *type) {
    write_code(index_gen_file, "\n\nstatic inline int index_gen_generate_mem_o2m_%s(struct index_gen_fw_index_%s *fw_index, size_t n, const char *filename) {\n", type, type);
    write_code(index_gen_file, "    qsort(fw_index, n, sizeof(struct index_gen_fw_index_%s), index_gen_fw_compar_%s);\n", type, type);
    write_code(index_gen_file, "    struct index_gen_fw_index_%s *fw = fw_index, *fw_end = fw + n;\n", type);
    write_code(index_gen_file, "    struct file_writer fw_idx = FILE_WRITER_INITIALIZER;\n");
    write_code(index_gen_file, "    if (0 > file_writer_init(&fw_idx, filename))\n");
    write_code(index_gen_file, "        return -1;\n");
    write_code(index_gen_file, "    struct index_header_o2m header;\n");
    write_code(index_gen_file, "    memset(&header, 0, sizeof(header));\n");
    write_code(index_gen_file, "    file_writer_write(&fw_idx, &header, sizeof(header));\n");
    write_code(index_gen_file, "    struct vmbuf vmb_keys = VMBUF_INITIALIZER;\n");
    write_code(index_gen_file, "    vmbuf_init(&vmb_keys, 1024*1024);\n");
    write_code(index_gen_file, "    uint32_t vect = 0;\n");
    write_code(index_gen_file, "    uint32_t num_keys = 0;\n");
    write_code(index_gen_file, "    for (; fw != fw_end; ++num_keys) {\n");
    write_code(index_gen_file, "        %s key = fw->key;\n", type);
    write_code(index_gen_file, "        uint32_t size = 0;\n");
    write_code(index_gen_file, "        for (; fw != fw_end && key == fw->key; ++fw, ++size) {\n");
    write_code(index_gen_file, "            file_writer_write(&fw_idx, &fw->row_loc, sizeof(uint32_t));\n");
    write_code(index_gen_file, "        }\n");
    write_code(index_gen_file, "        struct index_key_rec_o2m_%s output_rec = { key, size, vect };\n", type);
    write_code(index_gen_file, "        vmbuf_memcpy(&vmb_keys, &output_rec, sizeof(output_rec));\n");
    write_code(index_gen_file, "        vect += size;\n");
    write_code(index_gen_file, "    }\n");
    write_code(index_gen_file, "    uint64_t keys_ofs = file_writer_wlocpos(&fw_idx);\n");
    write_code(index_gen_file, "    file_writer_write(&fw_idx, vmbuf_data(&vmb_keys), vmbuf_wlocpos(&vmb_keys));\n");
    write_code(index_gen_file, "    vmbuf_free(&vmb_keys);\n");
    write_code(index_gen_file, "    size_t ofs = file_writer_wlocpos(&fw_idx);\n");
    write_code(index_gen_file, "    file_writer_lseek(&fw_idx, 0, SEEK_SET);\n");
    write_code(index_gen_file, "    header = (struct index_header_o2m){ IDX_O2M_SIGNATURE, keys_ofs, num_keys };\n");
    write_code(index_gen_file, "    file_writer_write(&fw_idx, &header, sizeof(header));\n");
    write_code(index_gen_file, "    file_writer_lseek(&fw_idx, ofs, SEEK_SET);\n");
    write_code(index_gen_file, "    file_writer_close(&fw_idx);\n");
    write_code(index_gen_file, "    return 0;\n");
    write_code(index_gen_file, "}");
}

void _index_gen_generate_ds_file(char * type) {
    write_code(index_gen_file, "\n\nstatic inline int _index_gen_generate_ds_file_%s(const char *base_path, const char *db, const char *table, const char *field, int (*coalesce_func)(struct index_gen_fw_index_%s *fw_index, size_t n, const char *filename)) {\n", type, type);
    write_code(index_gen_file, "    char output_filename[PATH_MAX];\n");
    write_code(index_gen_file, "    if (PATH_MAX <= snprintf(output_filename, PATH_MAX, \"%%s/%%s/%%s/%%s.idx\", base_path, db, table, field))\n");
    write_code(index_gen_file, "        return LOGGER_ERROR(\"filename too long\"), -1;\n");
    write_code(index_gen_file, "    DS_FIELD(%s) ds = DS_FIELD_INITIALIZER;\n", type);
    write_code(index_gen_file, "    char filename[PATH_MAX];\n");
    write_code(index_gen_file, "    // strlen(\"%%s/%%s/%%s/%%s\") < strlen(\"%%s/%%s/%%s/%%s.idx\"); not checking again\n");
    write_code(index_gen_file, "    snprintf(filename, PATH_MAX, \"%%s/%%s/%%s/%%s\", base_path, db, table, field);\n");
    write_code(index_gen_file, "    if (0 > DS_FIELD_INIT(%s, &ds, filename))\n", type);
    write_code(index_gen_file, "        return LOGGER_ERROR(\"failed to init datastore\"), -1;\n");
    write_code(index_gen_file, "\n");
    write_code(index_gen_file, "    struct vmbuf fw_idx = VMBUF_INITIALIZER;\n");
    write_code(index_gen_file, "    vmbuf_init(&fw_idx, sizeof(struct index_gen_fw_index_%s) * DS_FIELD_NUM_ELEMENTS(&ds));\n", type);
    write_code(index_gen_file, "    struct index_gen_fw_index_%s *fw = (struct index_gen_fw_index_%s *)vmbuf_data(&fw_idx);\n", type, type);
    write_code(index_gen_file, "    %s *rec_begin = DS_FIELD_BEGIN(&ds), *rec_end = DS_FIELD_END(&ds), *rec = rec_begin;\n", type);
    write_code(index_gen_file, "    for (; rec != rec_end; ++rec, ++fw) {\n");
    write_code(index_gen_file, "        fw->key = *rec;\n");
    write_code(index_gen_file, "        fw->row_loc = rec - rec_begin;\n");
    write_code(index_gen_file, "    }\n");
    write_code(index_gen_file, "    int res = coalesce_func((struct index_gen_fw_index_%s *)vmbuf_data(&fw_idx), rec - rec_begin, output_filename);\n", type);
    write_code(index_gen_file, "    DS_FIELD_FREE(%s, &ds);\n", type);
    write_code(index_gen_file, "    vmbuf_free(&fw_idx);\n");
    write_code(index_gen_file, "    return res;\n");
    write_code(index_gen_file, "}");
}

void index_gen_generate_ds_file_o2o(char *type) {
    write_code(index_gen_file, "\n\nstatic inline int index_gen_generate_ds_file_o2o_%s(const char *base_path, const char *db, const char *table, const char *field) {\n", type);
    write_code(index_gen_file, "    return _index_gen_generate_ds_file_%s(base_path, db, table, field,  index_gen_generate_mem_o2o_%s);\n", type, type);
    write_code(index_gen_file, "}");
}

void index_gen_generate_ds_file_o2m(char *type) {
    write_code(index_gen_file, "\n\nstatic inline int index_gen_generate_ds_file_o2m_%s(const char *base_path, const char *db, const char *table, const char *field) {\n", type);
    write_code(index_gen_file, "    return _index_gen_generate_ds_file_%s(base_path, db, table, field,  index_gen_generate_mem_o2m_%s);\n", type, type);
    write_code(index_gen_file, "}");
}

void ds_code_gen_index_gen(const char *filename) {
    if (!(index_gen_file = fopen(filename, "w")))
        die_perror("fopen");

    write_generated_file_comment(index_gen_file, __FILE__);
    write_code(index_gen_file, "#include <limits.h>");
    uint32_t i = 0;
    for (; i < sizeof(ds_types) / sizeof(ds_types[0]); i++) {
        struct_fw_index(ds_types[i]);
        struct_index_key_rec_o2o(ds_types[i]);
        struct_index_key_rec_o2m(ds_types[i]);
        index_gen_fw_compar(ds_types[i]);
        index_gen_generate_mem_o2o(ds_types[i]);
        index_gen_generate_mem_o2m(ds_types[i]);
        _index_gen_generate_ds_file(ds_types[i]);
        index_gen_generate_ds_file_o2o(ds_types[i]);
        index_gen_generate_ds_file_o2m(ds_types[i]);
    }

    if (0 != fclose(index_gen_file))
        die_perror("fclose");
}
