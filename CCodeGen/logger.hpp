#ifndef LOG_HPP_
#define LOG_HPP_

#include "mex.h"
#include "matrix.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "DRTB_modeldefHeader_tmp.hpp"

#define NUM_VARS 8
#define MAX_HISTORY 1000
#define MAX_VAR_LEN 30

/****************************************************************
 * MAX_FIELDS - The maximum number of fields in the structure
 * send to the mexFunction as program_options.
 *
 * NUM_OF_FIELDS - This is the current number of fields present
 * in the structure program_options.
 *****************************************************************/

#define MAX_FIELDS 20
#define NUM_OF_FIELDS 4

/* index for program_options structure elements */

#define PANIC_FILE_INDEX 0
#define PERIODIC_FILE_INDEX 1
#define MAX_HISTORY_INDEX 2
#define PERIOD_INDEX 3

/*Default values for the debugging */

#define DEFAULT_PANIC_FILE "panic_log.txt"
#define DEFAULT_PERIODIC_FILE "periodic_log.txt"
#define DEFAULT_MAX_HISTORY 100
#define DEFAULT_PERIOD 100


/* Assigns an index to variables */
enum VAR
{
	RAND_ONE = 0,
	RAND_TWO,
	T_CURR,
	T_NEXT,
	STATES,
	PROPENSITIES,
	CHOSEN_PROPENSITY,
	REACTION_INDEX
};

/*defines the different output destinations */
enum OUTPUT
{
	STD_OUTPUT = 0, FILE_OUTPUT
};

/* Defines the log level */
enum LOGLEVEL
{
	ALL = 0, DEBUG, INFO, OFF
};

bool shouldBeLogged(LOGLEVEL level, int *log_level_vars, VAR var);

void initializeLoggingFlags(LOGLEVEL level, int* log_level_of_vars,
		bool* logging_flag_of_var);

void update_logRotation(long long unsigned i, LOGLEVEL level,
		bool* logging_flag_of_var, double *log_rand_one, double *log_rand_two,
		double *log_t_curr, double* log_t_next,
		double log_states[][SSA_NumStates],
		double log_propensities[][SSA_NumReactions],
		double* log_choosen_propensities, double *log_reaction_indices,
		double curr_rand_one, double curr_rand_two, double curr_t_curr,
		double curr_t_next, double curr_states[], double curr_propensities[],
		double curr_choosen_propensity, double curr_reaction_index);

void writeOneStep(OUTPUT method, std::ofstream& fstream,
		long long unsigned current_step, LOGLEVEL level,
		bool* logging_flag_of_var, double log_rand_one, double log_rand_two,
		double log_t_curr, double log_t_next, double log_states[],
		double log_propensities[], double log_choosen_propensities,
		double log_reaction_indices);

void writeLastNSteps(OUTPUT destination, std::ofstream& fstream,
		long long unsigned currentHistoryStep, long long unsigned maxHistory,
		LOGLEVEL level, bool* logging_flag_of_var, double *log_rand_one,
		double *log_rand_two, double *log_t_curr, double* log_t_next,
		double log_states[][SSA_NumStates],
		double log_propensities[][SSA_NumReactions],
		double* log_choosen_propensities, double *log_reaction_indices);
//void writePeriodicLog(OUTPUT method, std::ofstream& fstream,
//		long long unsigned current_step, LOGLEVEL level,
//		bool* logging_flag_of_var, double log_rand_one, double log_rand_two,
//		double log_t_curr, double log_t_next, double log_states[],
//		double log_propensities[], double log_choosen_propensities,
//		double log_reaction_indices);
void openOutputStream(std::string file_name, std::ofstream& fstream);
void closeOutputStream(std::ofstream& fstream);

mxArray* getFieldPointer(const mxArray *struct_array, int index,
		const char* fieldName, mxClassID classIdExpected);

//void ssaCalloc(double **fieldPtr, mwSize n);
//void ssaCalloc(double ***fieldPtr, mwSize n , mwSize m );

#define CHECK_NOTNEG(x) if (x < 0) { mexPrintf("The propensity is : %lf",x);return -1;}
#endif
