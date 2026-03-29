#include "fitness.h"
#include "hospital.h"
typedef struct
{
    Hospital *hospitals;
    int size;
    Fitness fitness;
} Individual;