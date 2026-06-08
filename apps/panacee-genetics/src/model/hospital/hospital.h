#ifndef HOSPITAL_H
#define HOSPITAL_H

/**
 * @file hospital.h
 * @brief Hospital gene: a hospital placed in a town.
 */

/**
 * @brief A hospital placed in a town, identified by its INSEE code.
 */
typedef struct
{
    int insee;      /**< INSEE code of the town hosting the hospital. */
    int beds_count; /**< Number of beds (computed at the end of the run). */
} Hospital;

#endif
