#include "genetic.h"
#include "infrastructure/parser/parser.h"

int main(void)
{
    int i = 0;
    Town *towns = parse(&i);
    run_genetic(towns, i);

    return 0;
}
