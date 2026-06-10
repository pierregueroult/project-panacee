#ifndef PARSER_H
#define PARSER_H

/**
 * @file parser.h
 * @brief CSV parser for the towns dataset.
 */

#include "../../town/town.h"

/**
 * @brief Parse the towns CSV file.
 *
 * If path is NULL or empty, falls back to the default dataset path.
 *
 * @param path CSV file path, or NULL to use the default.
 * @param count Output: number of towns parsed (0 on failure).
 * @return Malloc'ed array of towns, or NULL on failure. Caller frees it.
 */
Town *parse(const char *path, int *count);

#endif
