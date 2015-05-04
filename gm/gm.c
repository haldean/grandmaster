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

#include <grandmaster/core.h>
#include <grandmaster/tree.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define GM_PORT ("7100")
#define MAX_MSG_LEN (4096)

static int sockfd = -1;

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
    str = malloc(msg_len);
    read_len = recv(sock, str, msg_len, 0);
    if (read_len < msg_len) {
        free(str);
        return NULL;
    }
    return str;
}

void
run_gm(struct game_tree *gt)
{
    (void)(gt);
    int err;
    int insock;
    char *msg;
    struct addrinfo *self;
    struct addrinfo hints;
    struct sockaddr inaddr;
    socklen_t inaddr_len;

    sockfd = -1;

    memset(&hints, 0x00, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, GM_PORT, &hints, &self);
    if (err) {
        fprintf(stderr, "E: getaddrinfo error: %s\n", gai_strerror(err));
        return;
    }

    sockfd = socket(self->ai_family, self->ai_socktype, self->ai_protocol);
    if (sockfd == -1) {
        perror("E: socket error");
        return;
    }

    err = bind(sockfd, self->ai_addr, self->ai_addrlen);
    if (err) {
        perror("E: bind error");
        return;
    }

    err = listen(sockfd, 10);
    if (err) {
        perror("E: listen error");
        return;
    }

    for (;;) {
        inaddr_len = sizeof(struct sockaddr);
        insock = accept(sockfd, &inaddr, &inaddr_len);
        if (insock == -1) {
            perror("E: accept error");
            goto close;
        }
        msg = read_str(insock, MAX_MSG_LEN);
        send(insock, "go away\n", 9, 0);
        close(insock);
    }

close:
    freeaddrinfo(self);
    if (sockfd != -1)
        close(sockfd);
}

void
handle_signal(int sig)
{
    int old_sockfd;
    switch (sig) {
    case SIGTERM:
        if (sockfd != -1) {
            /* set sockfd to -1 before closing it, so that the event loop
             * doesn't try to close the socket a second time immediately after
             * the call to accept fails. */
            old_sockfd = sockfd;
            sockfd = -1;
            close(old_sockfd);
        }
    }
}

int
main()
{
    struct game_tree gt;
    signal(SIGTERM, handle_signal);
    init_gametree(&gt);
    run_gm(&gt);
}
