/*
 * gmutil.c: utilities for grandmaster server code
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

#include <grandmaster/core.h>
#include <grandmaster/tree.h>

#include <jansson.h>
#include <stdlib.h>

#define MAX_MSG_LEN 16384

/* Receive a length-encoded string on the given socket. */
char *
read_str(int sock, ssize_t max_len);

/* Send a length-encoded string on the given socket. Returns 0 on success or -1
 * on error.*/
int
send_str(int sock, char *str);

json_t *
handle_new_game(struct game_tree *gt, json_t *req);

json_t *
handle_move(struct game_tree *gt, json_t *req);
