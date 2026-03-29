#include <stdio.h>
#include "parser.h"
#include "genetic.h"

int main(void)
{
    puts("Panacée: projet C initialisé.");
    int i = 0;
    Town *towns = parse(&i);

    run_genetic(towns, i);

    return 0;
}
