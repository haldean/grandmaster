#include "gameio.h"

void
read_location(const char *str, struct position *result)
{
    result->rank = str[0] - 'a';
    result->file = str[1] - '1';
}
