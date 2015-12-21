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
#include <limits.h>
#include <string.h>
#include "code_gen_ds_loader.h"
#include "logger.h"

#define WRITE_CODE(FILE, ...) \
    if (0 > write_code(FILE, __VA_ARGS__)) \
        return LOGGER_ERROR("write_code"), -1

int write_code(FILE *file, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    if (0 > vfprintf(file, format, ap))
        return LOGGER_ERROR("fprintf"), -1;
    va_end(ap);
    return 0;
}

char *ds_loader_type_to_str(ds_type_t type) {
    switch (type) {
    case ds_type_int8_t:
        return "int8_t";
    case ds_type_uint8_t:
        return "uint8_t";
    case ds_type_int16_t:
        return "int16_t";
    case ds_type_uint16_t:
        return "uint16_t";
    case ds_type_int32_t:
        return "int32_t";
    case ds_type_uint32_t:
        return "uint32_t";
    case ds_type_int64_t:
        return "int64_t";
    case ds_type_uint64_t:
        return "uint64_t";
    case ds_type_float:
        return "float";
    case ds_type_double:
        return "double";
    case ds_type_var:
        return "var";
    }
    LOGGER_ERROR("Unknown type %d", type);
    return NULL;
}

int ds_loader_init(struct ds_loader_code_gen *loader) {
    if (loader->file_c)
        return 0;

    char *typename = getenv("RIBS_DS_NAME");

    loader->table_name = NULL;
    char filename[PATH_MAX];
    if (PATH_MAX <= snprintf(filename, PATH_MAX, "%s.h", typename))
        return LOGGER_ERROR("filename too long"), ds_loader_close(loader), -1;
    if (!(loader->file_h = fopen(filename, "w")))
        return LOGGER_PERROR("fopen"), -1;
    if (PATH_MAX <= snprintf(filename, PATH_MAX, "%s.c", typename))
        return LOGGER_ERROR("filename too long"), ds_loader_close(loader), -1;
    if (!(loader->file_c = fopen(filename, "w")))
        return LOGGER_PERROR("fopen"), ds_loader_close(loader), -1;
    if (0 > vmbuf_init(&loader->file_list_buf, 0))
        return LOGGER_ERROR("vmbuf_init"), ds_loader_close(loader), -1;
    loader->typename = strdup(typename);

    WRITE_CODE(loader->file_h, "#ifndef _%s__H_\n", loader->typename);
    WRITE_CODE(loader->file_h, "#define _%s__H_\n", loader->typename);
    WRITE_CODE(loader->file_h, "\n");
    WRITE_CODE(loader->file_h, "#include \"ds.h\"\n");
    WRITE_CODE(loader->file_h, "#include \"idx.h\"\n");
    WRITE_CODE(loader->file_h, "\n");
    WRITE_CODE(loader->file_h, "typedef struct %s {\n", loader->typename);

    WRITE_CODE(loader->file_c, "#include \"%s.h\"\n", loader->typename);
    WRITE_CODE(loader->file_c, "#include \"vmbuf.h\"\n");
    WRITE_CODE(loader->file_c, "\n");
    WRITE_CODE(loader->file_c, "int %s_init(%s_t *ds_loader, const char *base_dir) {\n", loader->typename, loader->typename);
    WRITE_CODE(loader->file_c, "    int res = 0;\n");
    WRITE_CODE(loader->file_c, "    struct vmbuf vmb = VMBUF_INITIALIZER;\n");
    WRITE_CODE(loader->file_c, "    vmbuf_init(&vmb, 4096);\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "const char *%s_files[] = {\n", loader->typename);
    return 0;
}

int ds_loader_close(struct ds_loader_code_gen *loader) {
    if (!loader->file_c)
        return 0;

    WRITE_CODE(loader->file_h, "} %s_t;\n", loader->typename);
    WRITE_CODE(loader->file_h, "\n");
    WRITE_CODE(loader->file_h, "int %s_init(%s_t *ds_loader, const char *base_dir);\n", loader->typename, loader->typename);
    WRITE_CODE(loader->file_h, "\n");
    WRITE_CODE(loader->file_h, "extern const char *%s_files[];\n", loader->typename);
    WRITE_CODE(loader->file_h, "\n");
    WRITE_CODE(loader->file_h, "#endif\n");

    WRITE_CODE(loader->file_c, "ds_loader_done:\n");
    WRITE_CODE(loader->file_c, "    vmbuf_free(&vmb);\n");
    WRITE_CODE(loader->file_c, "    return res;\n");
    WRITE_CODE(loader->file_c, "}\n");
    WRITE_CODE(loader->file_c, "\n");
    WRITE_CODE(loader->file_c, "%.*s", vmbuf_wlocpos(&loader->file_list_buf), vmbuf_data(&loader->file_list_buf));
    WRITE_CODE(loader->file_c, "    NULL\n");
    WRITE_CODE(loader->file_c, "};\n");

    fclose(loader->file_h);
    fclose(loader->file_c);
    free(loader->typename);
    free(loader->db_name);
    if (loader->table_name)
        free(loader->table_name);
    vmbuf_free(&loader->file_list_buf);
    return 0;
}

void ds_loader_db(struct ds_loader_code_gen *loader, const char *db_name) {
    if (loader->db_name)
        free(loader->db_name);
    loader->db_name = strdup(db_name);
}

void ds_loader_table(struct ds_loader_code_gen *loader, const char *table_name) {
    if (loader->table_name)
        free(loader->table_name);
    loader->table_name = strdup(table_name);
}

#define DS_LOADER_FIELD_PARAMS loader->db_name, loader->table_name, name
#define DS_LOADER_FIELD_NAME "%s_%s_%s"
#define DS_LOADER_FIELD_PATH "%s/%s/%s"

int ds_loader_var_field(struct ds_loader_code_gen *loader, const char *name) {
    WRITE_CODE(loader->file_h, "    struct ds_var_field "DS_LOADER_FIELD_NAME";\n", DS_LOADER_FIELD_PARAMS);

    WRITE_CODE(loader->file_c, "    vmbuf_reset(&vmb);\n");
    WRITE_CODE(loader->file_c, "    vmbuf_sprintf(&vmb, \"%%s/"DS_LOADER_FIELD_PATH"\", base_dir);\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "    if (0 > (res = ds_var_field_init(&ds_loader->"DS_LOADER_FIELD_NAME", vmbuf_data(&vmb))))\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "        goto ds_loader_done;\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH"\",\n", DS_LOADER_FIELD_PARAMS);
    return 0;
}

int ds_loader_field(struct ds_loader_code_gen *loader, const char *name, ds_type_t type) {
    if (ds_type_var == type)
        return ds_loader_var_field(loader, name);

    char *type_str = ds_loader_type_to_str(type);
    WRITE_CODE(loader->file_h, "    struct ds_field_%s "DS_LOADER_FIELD_NAME";\n", type_str, DS_LOADER_FIELD_PARAMS);

    WRITE_CODE(loader->file_c, "    vmbuf_reset(&vmb);\n");
    WRITE_CODE(loader->file_c, "    vmbuf_sprintf(&vmb, \"%%s/"DS_LOADER_FIELD_PATH"\", base_dir);\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "    if (0 > (res = ds_field_%s_init(&ds_loader->"DS_LOADER_FIELD_NAME", vmbuf_data(&vmb))))\n", type_str, DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "        goto ds_loader_done;\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH"\",\n", DS_LOADER_FIELD_PARAMS);
    return 0;
}

int ds_loader_idx_o2o(struct ds_loader_code_gen *loader, const char *name, ds_type_t type) {
    char *type_str = ds_loader_type_to_str(type);
    WRITE_CODE(loader->file_h, "    struct index_container_o2o_%s "DS_LOADER_FIELD_NAME"_idx;\n", type_str, DS_LOADER_FIELD_PARAMS);

    WRITE_CODE(loader->file_c, "    vmbuf_reset(&vmb);\n");
    WRITE_CODE(loader->file_c, "    vmbuf_sprintf(&vmb, \"%%s/"DS_LOADER_FIELD_PATH".idx\", base_dir);\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "    if (0 > (res = index_container_o2o_init_%s(&ds_loader->"DS_LOADER_FIELD_NAME"_idx, vmbuf_data(&vmb))))\n", type_str, DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "        goto ds_loader_done;\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH".idx\",\n", DS_LOADER_FIELD_PARAMS);
    return 0;
}

int ds_loader_var_idx_o2m(struct ds_loader_code_gen *loader, const char *name) {
    WRITE_CODE(loader->file_h, "    struct var_index_container_o2m "DS_LOADER_FIELD_NAME"_idx;\n", DS_LOADER_FIELD_PARAMS);

    WRITE_CODE(loader->file_c, "    vmbuf_reset(&vmb);\n");
    WRITE_CODE(loader->file_c, "    vmbuf_sprintf(&vmb, \"%%s/"DS_LOADER_FIELD_PATH"\", base_dir);\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "    if (0 > (res = var_index_container_o2m_init(&ds_loader->"DS_LOADER_FIELD_NAME"_idx, vmbuf_data(&vmb))))\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "        goto ds_loader_done;\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH".keys\",\n", DS_LOADER_FIELD_PARAMS);
    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH".idx\",\n", DS_LOADER_FIELD_PARAMS);
    return 0;
}

int ds_loader_idx_o2m(struct ds_loader_code_gen *loader, const char *name, ds_type_t type) {
    if (ds_type_var == type)
        return ds_loader_var_idx_o2m(loader, name);

    char *type_str = ds_loader_type_to_str(type);
    WRITE_CODE(loader->file_h, "    struct index_container_o2m_%s "DS_LOADER_FIELD_NAME"_idx;\n", type_str, DS_LOADER_FIELD_PARAMS);

    WRITE_CODE(loader->file_c, "    vmbuf_reset(&vmb);\n");
    WRITE_CODE(loader->file_c, "    vmbuf_sprintf(&vmb, \"%%s/"DS_LOADER_FIELD_PATH".idx\", base_dir);\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "    if (0 > (res = index_container_o2m_init_%s(&ds_loader->"DS_LOADER_FIELD_NAME"_idx, vmbuf_data(&vmb))))\n", type_str, DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "        goto ds_loader_done;\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH".idx\",\n", DS_LOADER_FIELD_PARAMS);
    return 0;
}

int ds_loader_idx_o2o_ht(struct ds_loader_code_gen *loader, const char *name) {
    WRITE_CODE(loader->file_h, "    struct hashtable "DS_LOADER_FIELD_NAME"_idx;\n", DS_LOADER_FIELD_PARAMS);

    WRITE_CODE(loader->file_c, "    vmbuf_reset(&vmb);\n");
    WRITE_CODE(loader->file_c, "    vmbuf_sprintf(&vmb, \"%%s/"DS_LOADER_FIELD_PATH".idx\", base_dir);\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "    if (0 > (res = hashtable_open(&ds_loader->"DS_LOADER_FIELD_NAME"_idx, 0, vmbuf_data(&vmb), O_RDONLY)))\n", DS_LOADER_FIELD_PARAMS);
    WRITE_CODE(loader->file_c, "        goto ds_loader_done;\n");
    WRITE_CODE(loader->file_c, "\n");

    vmbuf_sprintf(&loader->file_list_buf, "    \""DS_LOADER_FIELD_PATH".idx\",\n", DS_LOADER_FIELD_PARAMS);
    return 0;
}
