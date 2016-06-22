#include "mex.h"
#include "DRTB_modeldefHeader_tmp.hpp"
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstddef>
#include <fstream>

// This macro enables file logging
//#define FLOGGING

// This macro enables command line logging
//#ifdef PROGRAM_DEBUG
//#define CLOGGING
//#include "logger.hpp"
//#endif

// Input:   xCurr (current state vector)
// Input:   tCurr (current time)
// Input:   tNext (next state report time)

// Output:  tCurr  (time after last reaction)
// Output:  XCurr  (state at tCurr)
// Output:  XSaved (state at last report time)

#ifdef LOGGING

#define NUM_VARS 8
#define MAX_REACTIONS 1000
#define MAX_STATES 1000
#define MAX_HISTORY 1000
enum VAR
{
	RAND_ONE = 0,
	RAND_TWO,
	T_CURR,
	T_NEXT,
	STATES,
	PROPENSITIES,
	CHOOSEN_PROPENSITY,
	REACTION_INDICIES
};

enum LOGLEVEL
{
	ALL = 0, DEBUG, INFO, OFF
};
bool shouldBeLogged(LOGLEVEL level, int *log_level_vars, VAR var)
{
	if (log_level_vars[var] >= level)
	{
		return true;
	}
	return false;
}
void initializeLoggingFlags(LOGLEVEL level,int* log_level_of_vars,bool* logging_flag_of_var)
{
	if (shouldBeLogged(level, log_level_of_vars, RAND_ONE) == true)
	{
		logging_flag_of_var[RAND_ONE] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, RAND_TWO) == true)
	{
		logging_flag_of_var[RAND_TWO] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, T_CURR) == true)
	{
		logging_flag_of_var[T_CURR] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, T_NEXT) == true)
	{
		logging_flag_of_var[T_NEXT] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, STATES) == true)
	{
		logging_flag_of_var[STATES] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, PROPENSITIES) == true)
	{
		logging_flag_of_var[PROPENSITIES] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, CHOOSEN_PROPENSITY) == true)
	{
		logging_flag_of_var[CHOOSEN_PROPENSITY] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, REACTION_INDICIES) == true)
	{
		logging_flag_of_var[REACTION_INDICIES] = true;
	}

}

void update_logRotation(int i, int maxHistory, LOGLEVEL level, int num_states,
		int num_reactions, bool* logging_flag_of_var, double *log_rand_one,
		double *log_rand_two, double *log_t_curr, double* log_t_next,
		double log_states[][MAX_HISTORY],
		double log_propensities[][MAX_HISTORY],
		double* log_choosen_propensities, double *log_reaction_indices,
		double curr_rand_one, double curr_rand_two, double curr_t_curr,
		double curr_t_next, double curr_states[MAX_STATES],
		double curr_propensities[MAX_REACTIONS], double curr_choosen_propensity,
		double curr_reaction_index)
{

	//std::cout<<"function : "<<__FUNCTION__<<"  level: "<< level << std::endl;
	if (logging_flag_of_var[RAND_ONE] == true)
	{
		log_rand_one[i] = curr_rand_one;
	}
	if (logging_flag_of_var[RAND_TWO] == true)
	{
		log_rand_two[i] = curr_rand_two;
	}
	if (logging_flag_of_var[T_CURR] == true)
	{
		log_t_curr[i] = curr_t_curr;
	}
	if (logging_flag_of_var[T_NEXT] == true)
	{
		log_t_next[i] = curr_t_next;
	}
	if (logging_flag_of_var[ STATES] == true)
	{
		for (int j = 0; j < num_states; j++)
		{
			log_states[j][i] = curr_states[j];
		}
	}
	if (logging_flag_of_var[PROPENSITIES] == true)
	{
		for (int j = 0; j < num_reactions; j++)
		{
			log_propensities[j][i] = curr_propensities[j];
		}
	}
	if (logging_flag_of_var[CHOOSEN_PROPENSITY] == true)
	{
		log_choosen_propensities[i] = curr_choosen_propensity;
	}
	if (logging_flag_of_var[REACTION_INDICIES] == true)
	{
		log_reaction_indices[i] = curr_reaction_index;
	}

}
void write_log_to_file( int i, int maxHistory, LOGLEVEL level, int num_states,
		int num_reactions, bool* logging_flag_of_var, double *log_rand_one,
		double *log_rand_two, double *log_t_curr, double* log_t_next,
		double log_states[MAX_STATES][MAX_HISTORY],
		double log_propensities[MAX_REACTIONS][MAX_HISTORY],
		double* log_choosen_propensities, double *log_reaction_indices)
{
	std::ofstream stream;
	stream.open("log.txt", std::ios_base::binary | std::ios_base::out);
	for(int k = 0; k<maxHistory; k++)
	{

		//std::cout<<"level: "<< level << std::endl;
		if (logging_flag_of_var[RAND_ONE] == true)
		{

			stream<<"rand one : " <<log_rand_one[k]<<std::endl;
		}
		if (logging_flag_of_var[RAND_TWO] == true)
		{
			//std::cout<<"rand two: "<<log_rand_one[k]<<std::endl;
			stream<<"rand two : " <<log_rand_two[k]<<std::endl;
		}
		if (logging_flag_of_var[T_CURR] == true)
		{
			stream<<"t_curr : "<<log_t_curr[i]<<std::endl;
		}
		if (logging_flag_of_var[T_NEXT] == true)
		{
			stream<<"t_next: "<<log_t_next[i]<<std::endl;
		}
		if (logging_flag_of_var[STATES] == true)
		{
			stream<<"state vector : ";
			for (int j = 0; j < num_states; j++)
			{
				stream<<log_states[j][i]<<"  ";
			}
			stream<<std::endl;
		}
		if (logging_flag_of_var[PROPENSITIES] == true)
		{
			stream<<"propensity vector : ";
			for (int j = 0; j < num_reactions; j++)
			{
				stream<<log_propensities[j][i]<<"  ";
			}
			stream<<std::endl;
		}
		if (logging_flag_of_var[CHOOSEN_PROPENSITY] == true)
		{
			stream<<"chosen propensity : "<<log_choosen_propensities[i]<<std::endl;
		}
		if (logging_flag_of_var[REACTION_INDICIES] == true)
		{
			stream<<"current reaction index : "<<log_reaction_indices[i]<<std::endl;
		}
		stream<<std::endl;

	}
	stream.close();

}
#endif

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

	/* Declare Inputs*/
	double *xCurr;
	double *parameters;
	double *timepoints;
	int numTimepts;

	/* Load input values from prhs */
	xCurr = mxGetPr(prhs[0]);
	parameters = mxGetPr(prhs[1]);
	timepoints = mxGetPr(prhs[2]);
	numTimepts = (int) mxGetScalar(prhs[3]);

	/* Declare IMs*/
	double cumProps[SSA_NumReactions];
	int reactionIndex;
	int iTime;
	double tCurr;
	double tNext;

	/* Declare Outputs*/
	double* timecourse;

	/* Create Outputs I */
	plhs[0] = mxCreateDoubleMatrix(SSA_NumStates * numTimepts, 1, mxREAL);
	timecourse = mxGetPr(plhs[0]);

	/* program debug */
//#ifdef PROGRAM_DEBUG
//	INITIALIZE_CLOG(DEBUG)
//#endif

	/* logging parameters */

#ifdef LOGGING
	//std::cout<<"logging is enabled"<<std::endl;
	LOGLEVEL level;
	/* memory allocation of variables */
#ifdef LEVEL_ALL

	double log_rand_one[MAX_HISTORY];
	double log_rand_two[MAX_HISTORY];
	double log_t_curr[MAX_HISTORY];
	double log_t_next[MAX_HISTORY];
	double log_current_states [SSA_NumStates][MAX_HISTORY];
	double log_propensities [SSA_NumReactions][MAX_HISTORY];
	double log_choosen_propensity [MAX_HISTORY];
	double log_reaction_index [MAX_HISTORY];

	/* set level */
	level = ALL;
	//std::cout<<"logging level : "<<level<<std::endl;

#elif LEVEL_DEBUG
	double *lag_rand_one = NULL;
	double *log_rand_two = NULL;
	double log_t_curr[MAX_HISTORY];
	double log_t_next[MAX_HISTORY];
	double log_current_states [SSA_NumStates][MAX_HISTORY];
	double log_propensities [SSA_NumReactions][MAX_HISTORY];
	double *log_choosen_propensity = NULL;
	double log_reaction_index [MAX_HISTORY];

	/* set level */
	level = DEBUG;
	//std::cout<<"logging level : "<<level<<std::endl;

#elif LEVEL_INFO
	double *log_rand_one = NULL;
	double *log_rand_two = NULL;
	double *log_t_curr = NULL;
	double *log_t_next = NULL;
	double log_current_states [SSA_NumStates][MAX_HISTORY];
	double log_propensities [SSA_NumReactions][MAX_HISTORY];
	double *log_choosen_propensity = NULL;
	double log_reaction_index [MAX_HISTORY];
	/* set level */
	level = INFO;
	//std::cout<<"logging level : "<<level<<std::endl;
#else
	double *log_rand_one = NULL;
	double *log_rand_two = NULL;
	double *log_t_curr = NULL;
	double *log_t_next = NULL;
	double *log_current_states = NULL;
	double *log_propensities =NULL;
	double *log_choosen_propensity = NULL;
	double *log_reaction_index = NULL;

	/* set level */
	level = OFF;
	//std::cout<<"logging level : "<<level<<std::endl;
#endif

	/*definition of log levels */
	/* INFO - { STATES,PROPENSITIES, REACTION_INDICIES} */
	/* DEBUG - {T_CURR ,T_NEXT , CHOOSEN_PROPENSITY }  + INFO */
	/* ALL - {RAND_ONE , RAND_TWO } + DEBUG */

	int log_level_of_var[NUM_VARS];

	log_level_of_var[RAND_ONE] = 0;
	log_level_of_var[RAND_TWO] = 0;
	log_level_of_var[T_CURR] = 1;
	log_level_of_var[T_NEXT] = 1;
	log_level_of_var[STATES] = 2;
	log_level_of_var[PROPENSITIES] = 2;
	log_level_of_var[CHOOSEN_PROPENSITY] = 1;
	log_level_of_var[REACTION_INDICIES] = 2;

	/* initialize the logging flag for variables */
	bool logging_flag_of_var[NUM_VARS];
	initializeLoggingFlags(level,log_level_of_var,logging_flag_of_var);

#endif
	/* Write initial conditions to output */
	iTime = 0;
	for (int i = 0; i < SSA_NumStates; i++)
	{
		timecourse[iTime * SSA_NumStates + i] = xCurr[i];
	}
	iTime++;
	tNext = timepoints[iTime];

	/* Start iteration*/
	tCurr = timepoints[0];
	tNext = timepoints[iTime];
	int everythingCounts = 0;

	while (tCurr < timepoints[numTimepts - 1])
	{
		// Debugging info - massive performance decrease
		double rand1 = std::max(1.0, (double) rand()) / (double) RAND_MAX;
		double rand2 = std::max(1.0, (double) rand()) / (double) RAND_MAX;

		/* Calculate cumulative propensities in one step*/
		calculateCumProps(cumProps, xCurr, parameters);
		/* the propensity of each reaction can be printed out here*/

		/* Sample reaction time*/
		tCurr = tCurr + 1 / cumProps[SSA_NumReactions - 1] * log(1 / rand1);

		// Debugging information
		//if(everythingCounts >= 50000)
		//{
		//  mexPrintf("CPP main calculation: tnow %.8fs | tnext %.8fs | 1/prop %.16f | log(1/prop) %.4f |rand %.4f | dt %.16f\n", tCurr, tNext,1/cumProps[SSA_NumReactions-1], log10(1/cumProps[SSA_NumReactions-1]),log(1/rand1),1/cumProps[SSA_NumReactions-1]*log(1/rand1));
		//  everythingCounts = 0;
		//}

		/* If time > time out, write next datapoint to output*/
		while (tCurr >= tNext && iTime < numTimepts)
		{
			// Debugging information
			// mexPrintf("This is bound to be good: tnow %.8fs | tnext %.8fs | 1/prop %.16f | log(1/prop) %.4f |rand %.4f | dt %.16f\n", tCurr, tNext,1/cumProps[SSA_NumReactions-1], log10(1/cumProps[SSA_NumReactions-1]),log(1/rand1),1/cumProps[SSA_NumReactions-1]*log(1/rand1));

			for (int i = 0; i < SSA_NumStates; i++)
			{
				//cIndex  =  iTime *  SSA_NumStates this will save the repeated calcuation

				timecourse[iTime * SSA_NumStates + i] = xCurr[i];
				//   mexPrintf(" %d",xCurr[i]);
			}
			//mexPrintf("\n");
			iTime++;
			tNext = timepoints[iTime];
		}

		/* Sample reaction index*/
		double chosenProp = rand2 * cumProps[SSA_NumReactions - 1];
		reactionIndex = 1;
		for (int i = 1; cumProps[i - 1] <= chosenProp; i++)
			reactionIndex = i + 1;

		/* Update xCurr */
		updateState(xCurr, reactionIndex);
		everythingCounts = everythingCounts + 1;
		if (everythingCounts > MAX_HISTORY)
		{
			everythingCounts = 0;
		}

#ifdef  LOGGING
		/* this call store the parameters of simulation which can used to print
		 * at later stage in case of any error
		 */
		if(level < OFF)
		{
			//std::cout<<"updating logs...\n"<<std::endl;
			update_logRotation(everythingCounts, MAX_HISTORY,level, SSA_NumStates ,
					SSA_NumReactions, logging_flag_of_var, log_rand_one,
					log_rand_two, log_t_curr,log_t_next,
					log_current_states,
					log_propensities,
					log_choosen_propensity, log_reaction_index,
					rand1, rand2, tCurr,
					tNext, xCurr,
					cumProps,
					chosenProp ,reactionIndex);
		}
#endif
	}
#ifdef LOGGING
//	std::cout<<"writing logs to file\n"<<std::endl;
//	std::cout<<"writing to file \n";

	write_log_to_file( everythingCounts, MAX_HISTORY,level, SSA_NumStates ,
			SSA_NumReactions, logging_flag_of_var, log_rand_one,
			log_rand_two, log_t_curr,log_t_next,
			log_current_states,
			log_propensities,
			log_choosen_propensity, log_reaction_index);

#endif

}

