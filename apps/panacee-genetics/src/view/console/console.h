#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * @file console.h
 * @brief Console (stdout/stderr) view: all user-facing textual output of the run.
 *
 * The controller calls these with plain data so no model or algorithm code
 * formats text itself.
 */

/**
 * @brief Print the RNG seed and where it came from.
 * @param source Origin of the seed (e.g. "time", "argument").
 * @param seed Seed value.
 */
void console_seed(const char *source, unsigned int seed);

/** @brief Announce that the coverage map precomputation starts. */
void console_coverage_start(void);

/** @brief Announce that the coverage map precomputation is done. */
void console_coverage_done(void);

/**
 * @brief Warn that the dataset population differs from the spec total.
 * @param data_total Total inhabitants found in the dataset.
 * @param spec_total Total inhabitants expected by the spec.
 */
void console_dataset_mismatch(int data_total, int spec_total);

/**
 * @brief Print a one-line summary of the current generation.
 * @param gen Generation number.
 * @param best_score Fitness score of the best individual.
 * @param hospital_count Number of hospitals in the best individual.
 * @param uhc_count Number of university hospitals (CHRU).
 * @param distant_count Number of inhabitants far from any hospital.
 * @param distant_percent Percentage of inhabitants far from any hospital.
 */
void console_generation(int gen, double best_score, int hospital_count, int uhc_count, int distant_count,
                        double distant_percent);

/**
 * @brief Report a stagnation event and the new mutation rate.
 * @param stagnation Number of generations without improvement.
 * @param mutation_rate Updated mutation rate.
 */
void console_stagnation(int stagnation, double mutation_rate);

/**
 * @brief Report that the run stops because the stagnation limit was reached.
 * @param stagnation_limit Maximum allowed generations without improvement.
 */
void console_stagnation_stop(int stagnation_limit);

/** @brief Announce that the local search starts. */
void console_local_search_start(void);

/** @brief Announce that the local search is done. */
void console_local_search_done(void);

/**
 * @brief Print the final result summary.
 * @param hospital_count Number of hospitals in the final solution.
 * @param total_beds Total number of beds.
 * @param covered_inhabitants Number of inhabitants covered by a hospital.
 * @param fitness_score Fitness score of the final solution.
 */
void console_final_result(int hospital_count, int total_beds, int covered_inhabitants, double fitness_score);

#endif
