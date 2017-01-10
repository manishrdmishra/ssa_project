#include "mex.h"
#include "matrix.h"
#include "DRTB_modeldefHeader_tmp.hpp"
#include "logger_tmp.hpp"
#include "gillespie_tmp.hpp"
#include "program_option_parser_tmp.hpp"
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <fstream>

// Input:   xCurr (current state vector)
// Input:   tCurr (current time)
// Input:   tNext (next state report time)

// Output:  tCurr  (time after last reaction)
// Output:  states  (state at tCurr)
// Output:  XSaved (state at last report time)

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

	/* Declare Inputs*/
	double *states;
	double *parameters;
	double *time_points;
	int time_points_count;

	/* Load input values from prhs array */

	/* Process the first input of prhs */
	states = mxGetPr(prhs[0]);

	/* Process the second input of prhs */
	parameters = mxGetPr(prhs[1]);
	/* Process the third input of prhs array */
	ProgramOptionsParser program_options(prhs[2]);
	program_options.parse();


	/* Process the fourth input of prhs array */
	time_points = mxGetPr(prhs[3]);

	/* Process the fifth input of prhs array */
	time_points_count = (int) mxGetScalar(prhs[4]);

	/* Declare IMs*/
	double cumProps[SSA_NumReactions];
	int reactionIndex;
	int iTime;
	double tCurr;
	double tNext;

	/* Declare Outputs*/
	double* timecourse;

	/* Create Outputs I */
	plhs[0] = mxCreateDoubleMatrix(SSA_NumStates * time_points_count, 1, mxREAL);
	timecourse = mxGetPr(plhs[0]);



	Logger *logger = NULL;

#ifdef LOGGING
	std::cout<<"logging flag is enabled..\n"<<std::endl;
	LOGLEVEL level = LOGLEVEL::OFF;
	/* set level */
#ifdef LEVEL_ALL

	level = LOGLEVEL::ALL;
	//logger.setLogLevel(ALL);

#elif LEVEL_DEBUG

	level = LOGLEVEL::DEBUG;
	//logger.setLogLevel(DEBUG);

#elif LEVEL_INFO

	level = LOGLEVEL::INFO;
	//logger.setLogLevel(INFO);

#else

	level = LOGLEVEL::OFF;
	//logger.setLogLevel(OFF);
	/* As level is off disable the logging flag */
	std::cout<<"disabling logging as logging flag is set to OFF\n"<<std::endl;
#undef LOGGING

#endif

    // if logging flag is enabled and log level is not set to OFF

    // intantiate and initialize logging parameters
	LoggingParameters logging_parameters;
    logging_parameters.panic_file_name_ = std::string(program_options.panic_file_name());
 	logging_parameters.periodic_file_name_ = std::string(program_options.periodic_file_name());
 	/*long long unsgined num_history = program_options.num_history();
 	 if (num_history > MAX_HISTORY)
    {
        mexWarnMsgIdAndTxt("SSA:programOptions:NUMOFHISTORYTOOBIG",
                           "The value provided of number of history is too big, changing it to maximum allowed value\n");
        num_history = MAX_HISTORY;
    }
    */
 	logging_parameters.num_history_ = program_options.num_history();
 	logging_parameters.logging_period_= program_options.period();

 	// instantiate logger
 	logger = new Logger(logging_parameters);
	logger.setLogLevel(level);
	logger.initializeLoggingLevelOfVar();
	/* initialize the logging flag for variables */
	logger.initializeLoggingFlags();

#endif

	// create simulation input structure
	SimulationParametersIn simulation_parameters_in;
	simulation_parameters_in.states_ = states;
	simulation_parameters_in.time_points_count_ = time_points_count;
	simulation_parameters_in.time_points_ = time_points;
	simulation_parameters_in.parameters_ = parameters;

	// create simulation output structure
	SimulationParametersOut simulation_parameters_out;
	simulation_parameters_out.timecourse_ = timecourse;

	omp_set_num_threads(program_options.num_threads());
/*#pragma omp parallel sections
	{
	#pragma omp section
		{

			mexPrintf("section 1 id = %d, \n", omp_get_thread_num());
		}
	#pragma omp section
		{

			mexPrintf("section 2 id = %d, \n", omp_get_thread_num());
		}
	#pragma omp section
		{

			mexPrintf("section 3 id = %d, \n", omp_get_thread_num());
		}
	}
 */

	// instantiate Gillespie object according to logging is enabled or disabled
	Gillespie* gillespie =  new GillespieBasic(logger);
	std::cout<<"running simulation..\n"<<std::endl;
	gillespie->runSimulation(simulation_parameters_in,simulation_parameters_out);

	/*free the allocated memory */
	//delete gillespie;
}

