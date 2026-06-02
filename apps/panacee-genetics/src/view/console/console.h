#ifndef CONSOLE_H
#define CONSOLE_H

/* Console (stdout/stderr) view: all user-facing textual output of the run.
   The controller calls these with plain data so no model or algorithm code
   formats text itself. */

void console_seed(const char *source, unsigned int seed);

void console_coverage_start(void);
void console_coverage_done(void);

void console_dataset_mismatch(int data_total, int spec_total);

void console_generation(int gen, double best_score, int hospital_count, int uhc_count, int distant_count,
                        double distant_percent);

void console_stagnation(int stagnation, double mutation_rate);
void console_stagnation_stop(int stagnation_limit);

void console_local_search_start(void);
void console_local_search_done(void);

void console_final_result(int hospital_count, int total_beds, int covered_inhabitants, double fitness_score);

#endif
