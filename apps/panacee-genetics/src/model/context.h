#ifndef CONTEXT_H
#define CONTEXT_H

#include "town/town.h"

/* Read-only "problem environment" shared by every genetic operator.
   It bundles the data that used to be passed as 5-6 separate arguments.
   The Context does NOT own these pointers: the controller builds them
   (parser, build_insee_to_idx, precompute_coverage) and frees them. */
typedef struct Context
{
    const Town *towns;            /* the dataset of towns                    */
    int         town_count;       /* number of towns                         */
    const int  *insee_to_idx;     /* INSEE code -> town index, -1 if absent  */
    int       **coverage;         /* coverage[j] = towns within reach of j   */
    const int  *coverage_size;    /* coverage_size[j] = length of coverage[j]*/
    int         total_inhabitants;/* reference population total              */
} Context;

#endif
