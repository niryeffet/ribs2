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
_RIBS_INLINE_ int ds_var_field_get(struct ds_var_field *dsvf, size_t index, char **ret_ptr, size_t *ret_size) {
    size_t *ofs_table = dsvf->ofs_table;
    size_t ofs = ofs_table[index];
    *ret_ptr = file_mapper_data(&dsvf->data) + ofs;
    *ret_size = ofs_table[index + 1] - ofs;
    return 0;
}

_RIBS_INLINE_ int ds_var_field_get_array_ptr(struct ds_var_field *dsvf, size_t index, void **ret_ptr, size_t *ret_size) {
    size_t *ofs_table = dsvf->ofs_table;
    size_t ofs = ofs_table[index];
    *ret_ptr = file_mapper_data(&dsvf->data) + ofs;
    *ret_size = ofs_table[index + 1] - ofs;
    return 0;
}

_RIBS_INLINE_ void *ds_var_field_get_array(struct ds_var_field *dsvf, size_t index, size_t *num_elements) {
    size_t *ofs_table = dsvf->ofs_table;
    size_t ofs = ofs_table[index];
    *num_elements = (ofs_table[index + 1] - ofs) / dsvf->array_el_size;
    return file_mapper_data(&dsvf->data) + ofs;
}

_RIBS_INLINE_ const char *ds_var_field_get_cstr(struct ds_var_field *dsvf, size_t index) {
    return file_mapper_data(&dsvf->data) + dsvf->ofs_table[index];
}

_RIBS_INLINE_ size_t ds_var_field_num_elements(struct ds_var_field *dsvf) {
    return dsvf->num_elements;
}

_RIBS_INLINE_ int ds_var_field_writer_new_row(struct ds_var_field_writer *dsvfw) {
    size_t ofs = file_writer_wlocpos(&dsvfw->data);
    *(size_t *)vmbuf_wloc(&dsvfw->ofs_table) = ofs;
    return vmbuf_wseek(&dsvfw->ofs_table, sizeof(size_t));
}
