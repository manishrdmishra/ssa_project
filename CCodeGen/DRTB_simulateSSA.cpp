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
	time_points = mxGetPr(prhs[2]);

	/* Process the fourth input of prhs array */
	time_points_count = (int) mxGetScalar(prhs[3]);

	/* Process the fifth input of prhs array */
	ProgramOptionsParser program_options_parser(prhs[4]);
	const ProgramOptions* program_options = program_options_parser.parse();


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

	// for parallelism
	omp_set_num_threads(program_options->num_threads());

	LOGLEVEL level;
#ifdef LOGGING
	std::cout<<"logging flag is enabled..\n"<<std::endl;
	/* set level */
#ifdef LEVEL_ALL
	level = ALL;
#elif LEVEL_DEBUG
	level = DEBUG;
#elif LEVEL_INFO
	level = INFO;
#else
	level = OFF;
	/* As level is off, disable the logging flag */
	std::cout<<"disabling logging as logging flag is set to OFF\n"<<std::endl;
#undef LOGGING
#endif

#endif

	LoggerFactory logger_factory(program_options);
	// if logging flag is enabled and log level is not set to OFF
	Logger *logger = NULL;
	if(level != OFF)
	{
		logger = logger_factory.create();
		logger->setLogLevel(level);
		/* initialize the logging flag for variables */
		logger->initializeLoggingFlags();

	}



	SimulationFactory simulation_factory(logger);
	// instantiate Gillespie object according to logging is enabled or disabled
	Gillespie* gillespie = simulation_factory.create(Gillespie::BASIC);

	// create simulation input structure
	SimulationParametersIn simulation_parameters_in;
	simulation_parameters_in.states_ = states;
	simulation_parameters_in.time_points_count_ = time_points_count;
	simulation_parameters_in.time_points_ = time_points;
	simulation_parameters_in.parameters_ = parameters;

	// create simulation output structure
	SimulationParametersOut simulation_parameters_out;
	simulation_parameters_out.timecourse_ = timecourse;
	std::cout<<"running simulation..\n"<<std::endl;
	gillespie->runSimulation(simulation_parameters_in,simulation_parameters_out);

	/*free the allocated memory */
	if( gillespie != NULL)
	{

		delete gillespie;
		gillespie = NULL;

	}
	if ( logger != NULL)
	{
		delete logger;
		logger = NULL;
	}
	if( program_options != NULL)
	{
		delete program_options;
		program_options = NULL;
	}

}

