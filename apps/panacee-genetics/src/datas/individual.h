#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include "fitness.h"
#include "hospital.h"

typedef struct
{
    Hospital *hospitals;
    int size;
    Fitness fitness;
} Individual;

#endif