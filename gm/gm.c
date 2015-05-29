/*
 * gm.c: grandmaster server
 * Copyright (C) 2015, Haldean Brown
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <string.h>

extern int client_main();
extern int server_main();

int
main(int argc, char *argv[])
{
    char *op_mode;
    if (argc < 2) {
        fprintf(stderr, "usage: gm [client|server]\n");
        return 1;
    }
    op_mode = argv[1];
    argc--; argv++;
    if (strcmp(op_mode, "client") == 0)
        return client_main();
    if (strcmp(op_mode, "server") == 0)
        return server_main(argc, argv);
    fprintf(stderr, "unrecognized operating mode %s\n", op_mode);
    return 1;
}
