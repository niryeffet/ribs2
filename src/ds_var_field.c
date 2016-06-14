/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2012,2013 Adap.tv, Inc.

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
#include "ds_var_field.h"
#include "ds_types.h"
#include "vmbuf.h"
#include "logger.h"
#include "ds.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct ds_var_field_header {
    int64_t type;
    size_t ofs_table;
    int64_t element_type; /* used when primary type is array */
};

int ds_var_field_init(struct ds_var_field *dsvf, const char *filename) {
    if (0 > ds_var_field_free(dsvf))
        return -1;
    if (0 > file_mapper_init(&dsvf->data, filename))
        return -1;
    size_t file_size = file_mapper_size(&dsvf->data);
    struct ds_var_field_header *header;
    if (file_size < sizeof(struct ds_var_field_header) ||
        (header = file_mapper_data(&dsvf->data))->ofs_table >= file_size)
        return LOGGER_ERROR("wrong size: %s", filename), file_mapper_free(&dsvf->data), -1;
    size_t ofs_table_size = file_size - header->ofs_table;
    if ((ofs_table_size & 7) > 0 ||
        ofs_table_size < 8)
        return LOGGER_ERROR_FUNC("%s: corrupted data detected", filename), file_mapper_free(&dsvf->data), -1;
    dsvf->num_elements = (ofs_table_size / sizeof(size_t)) - 1;
    dsvf->ofs_table = file_mapper_data(&dsvf->data) + header->ofs_table;
    size_t ofs_table = dsvf->ofs_table[dsvf->num_elements] + (size_t)7;
    ofs_table &= ~((size_t)7);
    if (ofs_table != header->ofs_table)
        return LOGGER_ERROR_FUNC("%s: corrupted data detected", filename), file_mapper_free(&dsvf->data), -1;
    if (ds_type_var != header->type && ds_type_array != header->type)
        return LOGGER_ERROR_FUNC("%s: corrupted data detected, wrong type or unsupported", filename), file_mapper_free(&dsvf->data), -1;
    dsvf->array_el_size = (ds_type_array == header->type ? ds_type_to_size(header->element_type) : 0);
    return 0;
}

int ds_var_field_free(struct ds_var_field *dsvf) {
    return file_mapper_free(&dsvf->data);
}

int ds_var_field_writer_init(struct ds_var_field_writer *dsvfw, const char *filename) {
    return ds_var_field_writer_init2(dsvfw, filename, ds_type_var, -1);
}

int ds_var_field_writer_init_array(struct ds_var_field_writer *dsvfw, const char *filename, int64_t element_type) {
    return ds_var_field_writer_init2(dsvfw, filename, ds_type_array, element_type);
}

int ds_var_field_writer_init2(struct ds_var_field_writer *dsvfw, const char *filename, int64_t type, int64_t element_type) {
    unlink(filename);
    if (0 > vmbuf_init(&dsvfw->ofs_table, 4096) ||
        0 > file_writer_init(&dsvfw->data, filename))
        return -1;
    struct ds_var_field_header header = {  -type, -1 , element_type }; /* set type to negative value so load fails if not finalized */
    if (0 > file_writer_write(&dsvfw->data, &header, sizeof(header)))
        return -1;
    return 0;
}

int ds_var_field_writer_write(struct ds_var_field_writer *dsvfw, const void *data, size_t data_size) {
    ds_var_field_writer_new_row(dsvfw);
    return file_writer_write(&dsvfw->data, data, data_size);
}

int ds_var_field_writer_append(struct ds_var_field_writer *dsvfw, const void *data, size_t data_size) {
    return file_writer_write(&dsvfw->data, data, data_size);
}

int ds_var_field_writer_close(struct ds_var_field_writer *dsvfw) {
    if (0 > ds_var_field_writer_new_row(dsvfw) ||
        0 > file_writer_align(&dsvfw->data))
        return -1;
    size_t ofs_table = file_writer_wlocpos(&dsvfw->data);
    if (0 > file_writer_write(&dsvfw->data, vmbuf_data(&dsvfw->ofs_table), vmbuf_wlocpos(&dsvfw->ofs_table)))
        return -1;

    size_t ofs_last = file_writer_wlocpos(&dsvfw->data);
    if (0 > file_writer_lseek(&dsvfw->data, 0, SEEK_SET))
        return -1;

    struct ds_var_field_header *header = (struct ds_var_field_header *)file_writer_wloc(&dsvfw->data);
    header->type = -header->type;
    header->ofs_table = ofs_table;
    if (0 > file_writer_lseek(&dsvfw->data, ofs_last, SEEK_SET) ||
        0 > file_writer_close(&dsvfw->data))
        return -1;
    vmbuf_free(&dsvfw->ofs_table);
    return 0;
}
