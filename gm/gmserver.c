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
#include <grandmaster/gmutil.h>

#include <jansson.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define GM_PORT ("7100")
#define MAX_MSG_LEN (4096)

static int sockfd = -1;

void
handle_client(
    struct game_tree *gt,
    int insock)
{
    char *req_msg;
    char *resp_msg;
    const char *req_kind;
    json_t *req;
    json_t *resp;
    json_t *t;
    json_error_t json_err;

    req = NULL;
    resp = NULL;

    req_msg = read_str(insock, MAX_MSG_LEN);
    if (req_msg == NULL) {
        fprintf(stderr, "I: unable to load message string\n");
        resp = json_pack("{ss}", "error", "couldn't load message string");
        goto close;
    }

    req = json_loads(req_msg, 0, &json_err);
    if (!req) {
        fprintf(stderr, "I: unable to parse json\n");
        resp = json_pack("{ss}", "error", "couldn't parse json");
        goto close;
    }

    t = json_object_get(req, "kind");
    if (t == NULL) {
        resp = json_pack("{ss}", "error", "no kind in request");
        goto close;
    }
    req_kind = json_string_value(t);

    resp = json_pack("{si}", "num_states", gt->n_states);

close:
    free(req_msg);
    if (req != NULL) {
        json_decref(req);
    }
    if (resp != NULL) {
        resp_msg = json_dumps(resp, JSON_PRESERVE_ORDER | JSON_INDENT(4));
        send_str(insock, resp_msg);
        free(resp_msg);
        json_decref(resp);
    }
    close(insock);
}

void
run_gm(struct game_tree *gt)
{
    int err;
    int insock;
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
        handle_client(gt, insock);
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
server_main()
{
    struct game_tree gt;
    signal(SIGTERM, handle_signal);
    init_gametree(&gt);
    run_gm(&gt);
    return 0;
}
