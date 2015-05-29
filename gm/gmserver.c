/*
 * gmserver.c: grandmaster server
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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define GM_PORT ("7100")

static int sockfd = -1;

bool
handle_client(
    struct game_tree *gt,
    int insock,
    FILE *aol)
{
    char *req_msg;
    char *resp_msg;
    const char *req_kind;
    size_t req_kind_len;
    size_t req_len;
    json_t *req;
    json_t *resp;
    json_t *t;
    json_error_t json_err;
    bool ok;

    req = NULL;
    resp = NULL;
    ok = false;

    req_msg = read_str(insock, MAX_MSG_LEN);
    if (req_msg == NULL) {
        fprintf(stderr, "I: unable to load message string\n");
        resp = json_pack("{ss}", "error", "couldn't load message string");
        goto close;
    }
    req_len = strlen(req_msg);

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
    req_kind_len = strnlen(req_kind, MAX_MSG_LEN);

    if (strncmp(req_kind, "new_game", req_kind_len) == 0) {
        resp = handle_new_game(gt, req);
    } else if (strncmp(req_kind, "move", req_kind_len) == 0) {
        resp = handle_move(gt, req);
    } else if (strncmp(req_kind, "game_from_pgn", req_kind_len) == 0) {
        resp = handle_game_from_pgn(gt, req);
    } else if (strncmp(req_kind, "end_game", req_kind_len) == 0) {
        resp = handle_end_game(gt, req);
    } else {
        resp = json_pack("{ss}", "error", "unknown kind");
    }

    t = json_object_get(resp, "error");
    if (json_string_value(t) == NULL) {
        /* +1 to account for null byte, which acts as a delimiter */
        fwrite(req, req_len + 1, 1, aol);
        ok = true;
    }

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
    return ok;
}

void
run_gm(struct game_tree *gt, FILE *aol)
{
    int err;
    int insock;
    struct addrinfo *self;
    struct addrinfo hints;
    struct sockaddr inaddr;
    socklen_t inaddr_len;
    struct aol_tx tx;

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
        tx = start_aol_tx(aol);
        if (handle_client(gt, insock, tx.f))
            commit_aol_tx(aol, tx);
        else
            cancel_aol_tx(aol, tx);
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
    case SIGINT:
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
server_main(int argc, char *argv[])
{
    struct game_tree gt;
    FILE *aol;
    char *aol_path;
    int res;

    if (argc < 2) {
        printf("usage: gm server path/to/append-only.log\n");
        return 1;
    }
    aol_path = argv[1];

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    aol = fopen(aol_path, "a+");
    if (aol == NULL) {
        perror("E: append-only log couldn't be opened");
        return 1;
    }

    init_gametree(&gt);
    res = load_aol(&gt, aol);
    if (res != 0)
        return res;
    run_gm(&gt, aol);
    return 0;
}
