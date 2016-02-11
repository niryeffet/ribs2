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
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "ds_code_gen_common.h"
#include "ds_code_gen_index_gen.h"
#include "ds_code_gen_index_container.h"

char *ds_types[] = {"int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t", "float", "double"};

void die(char *str) {
    printf("%s\n", str);
    exit(1);
}

void die_perror(char *str) {
    int BUF_SIZE = 1024;
    char buf[BUF_SIZE + 1];
    snprintf(buf, BUF_SIZE, "%s:%s", strerror(errno), str);
    die(buf);
}

void write_code(FILE *file, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    if (0 > vfprintf(file, format, ap))
        die("fprintf");
    va_end(ap);
}

void write_generated_file_comment(FILE *file, const char *src_file_name) {
    char full_path[PATH_MAX], *ribs_start;
    ribs_start = realpath(src_file_name, full_path);
    ribs_start = strstr(full_path, "/ribs2/");

    write_code(file, "/* THIS IS A GENERATED FILE. SOURCE CAN BE FOUND AT %s */\n\n", ribs_start + sizeof("/ribs2/") - 1);
    write_code(file, "/*\n"
"This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).\n"
"RIBS is an infrastructure for building great SaaS applications (but not\n"
"limited to).\n"
"\n"
"Copyright (C) 2012,2013,2014 Adap.tv, Inc.\n"
"\n"
"RIBS is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU Lesser General Public License as published by\n"
"the Free Software Foundation, version 2.1 of the License.\n"
"\n"
"RIBS is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU Lesser General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU Lesser General Public License\n"
"along with RIBS.  If not, see <http://www.gnu.org/licenses/>.\n"
"*/\n");
}

int main(int argc, char **argv) {
    if (4 > argc) {
        die("Usage: ds_code BASE_PATH GEN_FILE CONTAINER_FILE");
    }

    char *base_path = argv[1];
    char *gen_file = argv[2];
    char *container_file = argv[3];
    char filename[PATH_MAX];

    if (PATH_MAX <= snprintf(filename, PATH_MAX, "%s/%s", base_path, gen_file))
        die("filename too long");
    ds_code_gen_index_gen(filename);

    if (PATH_MAX <= snprintf(filename, PATH_MAX, "%s/%s", base_path, container_file))
        die("filename too long");
    ds_code_gen_index_container(filename);

    return 0;
}
