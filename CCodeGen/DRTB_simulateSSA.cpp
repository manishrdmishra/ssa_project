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

#ifdef LOGGING
#include "logger.hpp"
#endif

// Input:   xCurr (current state vector)
// Input:   tCurr (current time)
// Input:   tNext (next state report time)

// Output:  tCurr  (time after last reaction)
// Output:  XCurr  (state at tCurr)
// Output:  XSaved (state at last report time)

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

#ifdef LOGGING
	/* this should be declared in header file */
	int maxHistory = 10;

	LOGLEVEL level;

#ifdef LEVEL_ALL

	/* memory allocation of variables */
	double logRandOne[MAX_HISTORY];
	double logRandTwo[MAX_HISTORY];
	double logTCurr[MAX_HISTORY];
	double logTNext[MAX_HISTORY];
	double logCurrentStates [MAX_HISTORY][SSA_NumStates];
	double logPropensities [MAX_HISTORY][SSA_NumReactions];
	double logChosenPropensity [MAX_HISTORY];
	double logChosenReactionIndex [MAX_HISTORY];

	/* set level */
	level = ALL;

#elif LEVEL_DEBUG

	/* memory allocation of variables */
	double *lag_rand_one = NULL;
	double *log_rand_two = NULL;
	double log_t_curr[MAX_HISTORY];
	double log_t_next[MAX_HISTORY];
	double log_current_states [MAX_HISTORY][SSA_NumStates];
	double log_propensities [MAX_HISTORY][SSA_NumReactions];
	double *log_choosen_propensity = NULL;
	double log_reaction_index [MAX_HISTORY];

	/* set level */
	level = DEBUG;

#elif LEVEL_INFO

	/* memory allocation of variables */
	double *log_rand_one = NULL;
	double *log_rand_two = NULL;
	double *log_t_curr = NULL;
	double *log_t_next = NULL;
	double log_current_states [MAX_HISTORY][SSA_NumStates];
	double log_propensities [MAX_HISTORY][SSA_NumReactions];
	double *log_choosen_propensity = NULL;
	double log_reaction_index [MAX_HISTORY];
	/* set level */
	level = INFO;

#else

	/* memory allocation of variables */
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
	log_level_of_var[CHOSEN_PROPENSITY] = 1;
	log_level_of_var[REACTION_INDEX] = 2;

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
	int historyCounts = 0;
	int PERIOD = 20000;

	while (tCurr < timepoints[numTimepts - 1])
	{
		// Debugging info - massive performance decrease
		double rand1 = std::max(1.0, (double) rand()) / (double) RAND_MAX;
		double rand2 = std::max(1.0, (double) rand()) / (double) RAND_MAX;

		/* Calculate cumulative propensities in one step*/
		calculateCumProps(cumProps, xCurr, parameters);

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

#ifdef  LOGGING
		/* this call store the parameters of simulation which can used to print
		 * at later stage in case of any error
		 */

		if (historyCounts > maxHistory)
		{
			historyCounts = 0;
		}
		if(level < OFF)
		{
			//std::cout<<"updating logs...\n"<<std::endl;
			update_logRotation(historyCounts,level, logging_flag_of_var, logRandOne,
					logRandTwo, logTCurr,logTNext,
					logCurrentStates,
					logPropensities,
					logChosenPropensity, logChosenReactionIndex,
					rand1, rand2, tCurr,
					tNext, xCurr,
					cumProps,
					chosenProp ,reactionIndex);
		}

		everythingCounts = everythingCounts + 1;
		if (everythingCounts >= PERIOD)
		{
			/* the name of file should not be specified here
			 * it should come as parameter
			 */
			std::string file_name("periodic_log.txt");
			std::ofstream monitor_fstream;
			monitor_fstream.open(file_name.c_str(),
					std::ios_base::app | std::ios_base::ate | std::ios_base::out
					| std::ios_base::binary);

			writeOneStep(FILE_OUTPUT,monitor_fstream, level, logging_flag_of_var, rand1,
					rand2, tCurr,tNext,
					xCurr,cumProps,
					chosenProp, reactionIndex);
			monitor_fstream.close();
			everythingCounts = 0;

		}
#endif
	}

#ifdef LOGGING

	/* the name of file should not be specified here
	 * it should come as parameter
	 */
	std::string file_name("panic_log.txt");
	write_log(FILE_OUTPUT,file_name, everythingCounts, maxHistory,level, logging_flag_of_var, logRandOne,
			logRandTwo, logTCurr,logTNext,
			logCurrentStates,
			logPropensities,
			logChosenPropensity, logChosenReactionIndex);

#endif

}

