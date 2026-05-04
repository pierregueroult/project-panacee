#include "genetic.h"
#include "infrastructure/parser/parser.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int count = 0;
    Town *towns;
    Individual result;

    towns = parse(&count);
    if (!towns || count <= 0)
    {
        fprintf(stderr, "fatal: failed to load towns dataset\n");
        free(towns);
        return EXIT_FAILURE;
    }

    result = run_genetic(towns, count);

    free(result.hospitals);
    free(towns);
    return EXIT_SUCCESS;
}
