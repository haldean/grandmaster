/*
 * netutil.c: utilities for networking code
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

#include <grandmaster/gmutil.h>

#ifdef __linux__
#include <arpa/inet.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


char *
read_str(int sock, ssize_t max_len)
{
    ssize_t read_len;
    int32_t msg_len;
    char *str;

    read_len = recv(sock, &msg_len, sizeof(msg_len), 0);
    if (read_len < (ssize_t) sizeof(msg_len)) {
        return NULL;
    }
    msg_len = ntohl(msg_len);
    if (msg_len > max_len) {
        fprintf(stderr, "W: msg len %d too big: max len %ld. discarding.\n",
                msg_len, max_len);
        return NULL;
    }
    str = malloc(msg_len + 1);
    read_len = recv(sock, str, msg_len, 0);
    if (read_len < msg_len) {
        free(str);
        return NULL;
    }
    str[msg_len] = 0;
    return str;
}

int
send_str(int sock, char *str)
{
    ssize_t msg_len;
    ssize_t sent;
    ssize_t total_sent;
    uint32_t msg_len_n;

    msg_len = strlen(str);
    msg_len_n = htonl(msg_len);
    sent = send(sock, &msg_len_n, sizeof(msg_len_n), 0);
    if (sent == -1) {
        perror("E: failed to send message");
    }
    if (sent != sizeof(msg_len_n)) {
        fprintf(stderr, "E: failed to send message header");
        return -1;
    }

    total_sent = 0;
    while (total_sent < msg_len) {
        sent = send(sock, str + total_sent, msg_len - total_sent, 0);
        if (sent == -1) {
            perror("E: failed to send message");
        }
        total_sent += sent;
    }
    return 0;
}
