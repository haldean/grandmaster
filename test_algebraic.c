#include "grandmaster.h"
#include "gameio.h"

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    struct move *last;
    struct move *next;
    int i;
    char *notation;

    last = calloc(1, sizeof(struct move));
    get_root(last);
    print_move(last);

    for (i = 1; i < argc; i++) {
        notation = argv[i];
        parse_algebraic(notation, last, &next);
        if (next == NULL) {
            printf("FAILED: parse_algebraic returned error on %s\n", notation);
            return 1;
        }
        printf("\n");
        print_move(next);
        last = next;
    }

    return 0;
}
