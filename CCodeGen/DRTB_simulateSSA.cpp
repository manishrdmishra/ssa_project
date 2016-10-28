#include "mex.h"
#include "matrix.h"
#include "DRTB_modeldefHeader_tmp.hpp"
#include "logger_tmp.hpp"
#include "gillespie_tmp.hpp"
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstddef>
#include <fstream>
#include <cstring>
#include <limits>

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

	/****************************************************************
	 * mexFunction receives a structure in prhs[2].
	 * This structure provide the options related to debugging.
	 *  Currently this structure keep four elements.
	 *
	 *  program_options
	 * 		 panic_file_name - string
	 * 		 periodic_file_name - string
	 * 		 max_history  - uint64
	 * 		 period - unit64
	 *
	 *  panic_file_name - This string variable contains the name of file,
	 *  				  which stores the state of the simulation when an error
	 *  				  occurs and simulation can not proceed further.
	 *
	 *  periodic_file_name  - This string variable contains the name of file,
	 *  					  which stores the simulation states periodically.
	 *
	 *  max_history - When simulation runs then we store the last n states of
	 *  			  the simulation. When some error occurs then we print
	 *  			  these last n states in to panic_file_name. So n is assigned
	 *  			  the value provided by max_history.
	 *
	 *  period      -  After x steps the state of the simulation is written to
	 *  			   a file periodic_file_name. so x is assigned the value
	 *  			   provided by period.
	 *
	 * *****************************************************************/

	char *panic_file_name = NULL;
	char *periodic_file_name = NULL;
	long long unsigned *num_history = NULL;
	long long unsigned *period = NULL;
	bool assignedDefault[NUM_OF_FIELDS] =
			{ false };

	/* pointer to field names */
	const char **fnames;

	int num_fields;
	mwSize num_of_elements_in_structure;
	mxArray *fields[MAX_FIELDS];

	const mxArray *struct_array = prhs[2];

	/* Pre assign the class of each element present in the structure */
	mxClassID classIDflags[] =
			{ mxCHAR_CLASS, mxCHAR_CLASS, mxUINT64_CLASS, mxUINT64_CLASS };

	/* check if the input in struct_array is a structure */
	if (mxIsStruct(struct_array) == false)
	{
		mexErrMsgIdAndTxt("SSA:programOptions:inputNotStruct",
						  "Input must be a structure.");
	}

	/* get number of elements in structure */
	num_fields = mxGetNumberOfFields(struct_array);
	//mexPrintf("Number of fields provided in structure %d \n", num_fields);
	if (num_fields != NUM_OF_FIELDS)
	{
		mexWarnMsgIdAndTxt("SSA:programOptions:NumOfStructElementMismatch",
						   "The expected number of elements in structure does not match with the provided\n");
		//mexPrintf("Expected vs Provided : %d  %d\n", NUM_OF_FIELDS, num_fields);
	}

	num_of_elements_in_structure = mxGetNumberOfElements(struct_array);
	//mexPrintf("Number of elements in structure %d \n", num_of_elements_in_structure);
	/* allocate memory  for storing pointers */
	fnames = (const char**) mxCalloc(num_fields, sizeof(*fnames));

	/* get field name pointers */
	for (int i = 0; i < num_fields; i++)
	{
		fnames[i] = mxGetFieldNameByNumber(struct_array, i);

	}

	/* get the panic file name*/
	mxArray *panic_file = getFieldPointer(struct_array, 0,
										  fnames[PANIC_FILE_INDEX], classIDflags[0]);
	if (panic_file != NULL)
	{

		panic_file_name = mxArrayToString(panic_file);
		//mexPrintf("panic file name %s \n", panic_file_name);

	}
	else
	{
		int buflen = strlen(DEFAULT_PANIC_FILE) + 1;
		panic_file_name = (char *) mxCalloc(buflen, sizeof(char));
		strcpy(panic_file_name, DEFAULT_PANIC_FILE);
		//mexPrintf("default panic file name %s \n", panic_file_name);
		assignedDefault[PANIC_FILE_INDEX] = true;
	}

	/* get the periodic file name*/
	mxArray *periodic_file = getFieldPointer(struct_array, 0,
											 fnames[PERIODIC_FILE_INDEX], classIDflags[1]);
	if (periodic_file != NULL)
	{

		periodic_file_name = mxArrayToString(periodic_file);
		//mexPrintf("periodic file name %s \n", periodic_file_name);

	}
	else
	{
		int buflen = strlen(DEFAULT_PERIODIC_FILE) + 1;
		periodic_file_name = (char *) mxCalloc(buflen, sizeof(char));
		strcpy(periodic_file_name, DEFAULT_PERIODIC_FILE);
		//mexPrintf("default periodic file name %s \n", periodic_file_name);
		assignedDefault[PERIODIC_FILE_INDEX] = true;

	}

	/* get the max history value */
	mxArray *num_history_pointer = getFieldPointer(struct_array, 0,
												   fnames[NUM_HISTORY_INDEX], classIDflags[2]);

	if (num_history_pointer != NULL)
	{

		num_history = (long long unsigned*) mxGetData(num_history_pointer);
		mexPrintf("num history value %llu \n", *num_history);
	}
	else
	{
		num_history = (long long unsigned *) mxCalloc(1,
													  sizeof(long long unsigned));
		*num_history = DEFAULT_NUM_HISTORY;
		//mexPrintf("default max history value %llu \n", *num_history);
		assignedDefault[NUM_HISTORY_INDEX] = true;
	}

	/* sanity check - maxHistroy should be less than MAX_HISTORY */
	if (*num_history > MAX_HISTORY)
	{
		*num_history = MAX_HISTORY;
	}

	/* get the period value */
	mxArray *period_pointer = getFieldPointer(struct_array, 0,
											  fnames[PERIOD_INDEX], classIDflags[3]);

	if (period_pointer != NULL)
	{

		period = (long long unsigned *) mxGetPr(period_pointer);
		mexPrintf("period value %llu \n", *period);
	}
	else
	{
		period = (long long unsigned *) mxCalloc(1, sizeof(long long unsigned));
		*period = DEFAULT_PERIOD;
		//mexPrintf("default period value %llu \n", *period);
		assignedDefault[PERIOD_INDEX] = true;
	}

	/* free the memory */
	mxFree((void *) fnames);

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

#ifdef LOGGING
    LoggingParameters logging_parameters;
 	logging_parameters.panic_file_name_ = std::string(panic_file_name);
 	logging_parameters.periodic_file_name_ = std::string(periodic_file_name);
 	logging_parameters.num_history_ = *num_history;
 	logging_parameters.logging_period_= *period;
 	Logger logger(logging_parameters);
 	logger.initializeLoggingLevelOfVar();

	std::cout<<"logging enabled..\n"<<std::endl;


	/* set level */
#ifdef LEVEL_ALL

	//level = ALL;
	logger.setLogLevel(ALL);

#elif LEVEL_DEBUG

	//level = DEBUG;
	logger.setLogLevel(DEBUG);

#elif LEVEL_INFO

	//level = INFO;
	logger.setLogLevel(INFO);

#else

	//level = OFF;
	logger.setLogLevel(OFF);
	/* As level is off disable the logging flag */
	std::cout<<"disabling logging as logging flag is set to OFF\n"<<std::endl;
#undef LOGGING

#endif

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


	GillespieBasic* gillespie =  new GillespieBasic(logger);
	std::cout<<"starting simulation..\n"<<std::endl;
	gillespie->runSimulation(simulation_parameters_in,simulation_parameters_out);


	/*free the allocated memory */
	if (assignedDefault[PANIC_FILE_INDEX] == true)
	{
		mxFree((void*) panic_file_name);
	}
	if (assignedDefault[PERIODIC_FILE_INDEX] == true)
	{
		mxFree((void*) periodic_file_name);
	}
	if (assignedDefault[NUM_HISTORY_INDEX] == true)
	{
		mxFree((void*) num_history);
	}
	if (assignedDefault[PERIOD_INDEX] == true)
	{
		mxFree((void *) period);
	}
}

