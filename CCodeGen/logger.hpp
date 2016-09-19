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

/****************************************************************
 * In the simulation process there are variables which can be 
 * logged periodically or when some error occur (like propensity 
 *  has become negatvie ) during simulation. By logging these
 * variables value in a file we can trace back the source of error
 * or we can anaylyse how state of the system has changed over the 
 * time. 
 * We have identified some important variable which will be logged
 * They are { tCurr, tNext, rand1, rand2, xCurr,cumProps,
 * chosenProp, reactionIndex }. 
 *****************************************************************/

/* Number of variable which are identified for logging */
#define NUM_VARS 8

/* After some error this is the maximum number of steps
 * which will be printed to panic file 
 */
#define MAX_HISTORY 1000

/* This is the maximum number of character in the 
 * name of a varible. 
 */
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

/* index for program_options structure elements 
 * ie. the first field is fixed for the name of panic file
 * , second field is fixed for periodic file name and so on.
 */

#define PANIC_FILE_INDEX 0
#define PERIODIC_FILE_INDEX 1
#define NUM_HISTORY_INDEX 2
#define PERIOD_INDEX 3

/*Default values for the debugging */

#define DEFAULT_PANIC_FILE "panic_log.txt"
#define DEFAULT_PERIODIC_FILE "periodic_log.txt"
#define DEFAULT_NUM_HISTORY 100
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

#define CHECK_NOTNEG(reaction_id,propensity) if (propensity < 0) { mexPrintf("For reaction : %d the propensity is : %lf\n",reaction_id,propensity);return -1;}
#endif
