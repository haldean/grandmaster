/*
 * gmclient.c: grandmaster client
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

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define GM_PORT ("7100")
#define MAX_MSG_LEN (4096)

int
client_main()
{
    int err;
    int msglen;
    int sockfd;
    char *buf;
    struct addrinfo *self;
    struct addrinfo hints;
    char msg[MAX_MSG_LEN];

    msglen = read(STDIN_FILENO, msg, MAX_MSG_LEN - 1);
    msg[msglen++] = 0;

    memset(&hints, 0x00, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo("localhost", GM_PORT, &hints, &self);
    if (err) {
        fprintf(stderr, "E: getaddrinfo error: %s\n", gai_strerror(err));
        return -1;
    }

    sockfd = socket(self->ai_family, self->ai_socktype, self->ai_protocol);
    if (sockfd == -1) {
        perror("E: socket error");
        return -1;
    }

    err = connect(sockfd, self->ai_addr, self->ai_addrlen);
    if (err) {
        perror("E: connect error");
        return -1;
    }

    send_str(sockfd, msg);
    buf = read_str(sockfd, MAX_MSG_LEN);

    if (!buf) {
        fprintf(stderr, "E: got no response\n");
        return -1;
    }
    fprintf(stderr, "OK\n");
    printf("%s\n", buf);
    return 0;
}
