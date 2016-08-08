#include "mex.h"
#include "matrix.h"
#include "DRTB_modeldefHeader_tmp.hpp"
#ifdef LOGGING
#include "logger_tmp.hpp"
#endif
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstddef>
#include <fstream>
#include <cstring>

#define MAX_FIELDS 20
#define NUM_OF_FIELDS 4

/* index for structure elements */

#define PANIC_FILE_INDEX 0
#define PERIODIC_FILE_INDEX 1
#define MAX_HISTORY_INDEX 2
#define PERIOD_INDEX 3

/*Default values for the debugging */

#define DEFAULT_PANIC_FILE "panic_log.txt"
#define DEFAULT_PERIODIC_FILE "periodic_log.txt"
#define DEFAULT_MAX_HISTORY 100
#define DEFAULT_PERIOD 100

/* function  declarations */
mxArray* getFieldPointer(const mxArray *struct_array, int index,
		const char* fieldName, mxClassID classIdExpected);

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

	/* Get the program related options
	 * from the structure.
	 */
	char *panicFileName = NULL;
	//char defaultPanicFileName[] = "panic_log.txt";

	char *periodicFileName = NULL;
	//char defaultperiodicFileName[] = "periodic_log.txt";
	long long unsigned *maxHistory = NULL;
	long long unsigned *period = NULL;
	bool assignedDefault[NUM_OF_FIELDS] =
	{ false };
	/* pointer to field names */
	const char **fnames;

	int numFields;
	mwSize NStructElems;
	mxArray *fields[MAX_FIELDS];

	const mxArray *struct_array = prhs[2];
	mxClassID classIDflags[] =
	{ mxCHAR_CLASS, mxCHAR_CLASS, mxUINT64_CLASS, mxUINT64_CLASS };

	/* check if the input in struct_array is a structure */
	if (mxIsStruct(struct_array) == false)
	{
		mexErrMsgIdAndTxt("SSA:programOptions:inputNotStruct",
				"Input must be a structure.");
	}

	/* get number of elements in structure */
	numFields = mxGetNumberOfFields(struct_array);
	mexPrintf("Number of fields provided in structure %d \n", numFields);
	if (numFields != NUM_OF_FIELDS)
	{
		mexWarnMsgIdAndTxt("SSA:programOptions:NumOfStructElementMismatch",
				"The expected number of elements in structure does not match with the provided\n");
		mexPrintf("Expected vs Provided : %d  %d\n", NUM_OF_FIELDS, numFields);
	}

	NStructElems = mxGetNumberOfElements(struct_array);
	mexPrintf("Number of elements in structure %d \n", NStructElems);
	/* allocate memory  for storing pointers */
	fnames = (const char**) mxCalloc(numFields, sizeof(*fnames));

	/* get field name pointers */
	for (int i = 0; i < numFields; i++)
	{
		fnames[i] = mxGetFieldNameByNumber(struct_array, i);

	}

	/* get the panic file name*/
	mxArray *panic_file = getFieldPointer(struct_array, 0,
			fnames[PANIC_FILE_INDEX], classIDflags[0]);
	if (panic_file != NULL)
	{

		panicFileName = mxArrayToString(panic_file);
		mexPrintf("panic file name %s \n", panicFileName);

	}
	else
	{
		int buflen = strlen(DEFAULT_PANIC_FILE) + 1;
		panicFileName = (char *) mxCalloc(buflen, sizeof(char));
		strcpy(panicFileName, DEFAULT_PANIC_FILE);
		mexPrintf("default panic file name %s \n", panicFileName);
		assignedDefault[PANIC_FILE_INDEX] = true;
	}

	/* get the periodic file name*/
	mxArray *periodic_file = getFieldPointer(struct_array, 0,
			fnames[PERIODIC_FILE_INDEX], classIDflags[1]);
	if (periodic_file != NULL)
	{

		periodicFileName = mxArrayToString(periodic_file);
		mexPrintf("periodic file name %s \n", periodicFileName);

	}
	else
	{
		int buflen = strlen(DEFAULT_PERIODIC_FILE) + 1;
		periodicFileName = (char *) mxCalloc(buflen, sizeof(char));
		strcpy(periodicFileName, DEFAULT_PERIODIC_FILE);
		mexPrintf("default periodic file name %s \n", periodicFileName);
		assignedDefault[PERIODIC_FILE_INDEX] = true;

	}

	/* get the max history value */
	mxArray *max_history_pointer = getFieldPointer(struct_array, 0,
			fnames[MAX_HISTORY_INDEX], classIDflags[2]);

	if (max_history_pointer != NULL)
	{

		maxHistory = (long long unsigned*) mxGetData(max_history_pointer);
		mexPrintf("max history value %llu \n", *maxHistory);
	}
	else
	{
		maxHistory = (long long unsigned *) mxCalloc(1,
				sizeof(long long unsigned));
		*maxHistory = DEFAULT_MAX_HISTORY;
		mexPrintf("default max history value %llu \n", *maxHistory);
		assignedDefault[MAX_HISTORY_INDEX] = true;
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
		mexPrintf("default period value %llu \n", *period);
		assignedDefault[PERIOD_INDEX] = true;
	}

	/* free the memory */
	mxFree((void *) fnames);

	timepoints = mxGetPr(prhs[3]);
	numTimepts = (int) mxGetScalar(prhs[4]);

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
	/* Panic log file name */
	std::string panic_file_name(panicFileName);
	std::ofstream panic_fstream;

	/* periodic log file */
	std::string periodic_file_name(periodicFileName);
	std::ofstream periodic_fstream;
	openOutputStream(periodic_file_name, periodic_fstream);

	//std::cout<<"logging enabled...\n"<<std::endl;

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
	long long unsigned globalCounter = 0;
	long long unsigned historyCounts = 0;

	while (tCurr < timepoints[numTimepts - 1])
	{
		// Debugging info - massive performance decrease
		double rand1 = std::max(1.0, (double) rand()) / (double) RAND_MAX;
		double rand2 = std::max(1.0, (double) rand()) / (double) RAND_MAX;

		/* Calculate cumulative propensities in one step*/
		int retVal = calculateCumProps(cumProps, xCurr, parameters);
		//retVal = -1;
		if (retVal == -1)
		{
#ifdef LOGGING
			openOutputStream(panic_file_name, panic_fstream);
			writeLastNSteps(FILE_OUTPUT,panic_fstream, historyCounts, *maxHistory,level, logging_flag_of_var, logRandOne,
					logRandTwo, logTCurr,logTNext,
					logCurrentStates,
					logPropensities,
					logChosenPropensity, logChosenReactionIndex);

#endif
			mexErrMsgIdAndTxt("SSA:InvalidPropensity",
					"Propensity can not be negative");
		}

		/* Sample reaction time*/
		double temp = cumProps[SSA_NumReactions - 1] * log(1 / rand1);
		if (temp <= 0)
		{
#ifdef LOGGING

			openOutputStream(panic_file_name, panic_fstream);
			writeLastNSteps(FILE_OUTPUT,panic_fstream, historyCounts, *maxHistory,level, logging_flag_of_var, logRandOne,
					logRandTwo, logTCurr,logTNext,
					logCurrentStates,
					logPropensities,
					logChosenPropensity, logChosenReactionIndex);

#endif
			mexErrMsgIdAndTxt("SSA:InvalidTcurr",
					"Value of tCurr can not be negative");
		}
		tCurr = tCurr + 1 / temp;

		/* If time > time out, write next datapoint to output*/
		while (tCurr >= tNext && iTime < numTimepts)
		{

			// this will save the repeated calculation
			int cIndex = iTime * SSA_NumStates;
			for (int i = 0; i < SSA_NumStates; i++)
			{

				timecourse[cIndex + i] = xCurr[i];
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

		//std::cout<<"updating logs...\n"<<std::endl;
		globalCounter = globalCounter + 1;
#ifdef  LOGGING
		/* this call store the parameters of simulation which can used to print
		 * at later stage in case of any error
		 */
		historyCounts = historyCounts + 1;
		if (historyCounts > *maxHistory)
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

		//std::cout<<"global count: "<<globalCounter<<"\n";

		if (globalCounter % *period == 0)
		{

			//mexPrintf("printing logs..");
			writeOneStep(FILE_OUTPUT,periodic_fstream,globalCounter, level, logging_flag_of_var, rand1,
					rand2, tCurr,tNext,
					xCurr,cumProps,
					chosenProp, reactionIndex);

		}
#endif
	}
#ifdef LOGGING
	panic_fstream.close();
	periodic_fstream.close();
#endif

	/*free the allocated memory */
	if (assignedDefault[PANIC_FILE_INDEX] == true)
	{
		mxFree((void*) panicFileName);
	}
	if (assignedDefault[PERIODIC_FILE_INDEX] == true)
	{
		mxFree((void*) periodicFileName);
	}
	if (assignedDefault[MAX_HISTORY_INDEX] == true)
	{
		mxFree((void*) maxHistory);
	}
	if (assignedDefault[PERIOD_INDEX] == true)
	{
		mxFree((void *) period);
	}
}

/**************************************************************************
 * input : struct_arrary ( a  pointer to Matlab structure )
 * input : index ( index of an element in structure )
 * input : fieldName ( name of the element )
 * input : classIdExpected ( expected class id of the element)
 * output : pointer to the corresponding structure element
 *
 * This function takes the above input arguments checks the validity of
 * the structure element and if everything is fine then returns a pointer.
 **************************************************************************/
mxArray* getFieldPointer(const mxArray *struct_array, int index,
		const char* fieldName, mxClassID classIdExpected)
{
	mxArray *fieldPointer = NULL;
	//mexPrintf("executing getFieldPointer..");
	fieldPointer = mxGetField(struct_array, index, fieldName);
	if (fieldPointer == NULL || mxIsEmpty(fieldPointer))
	{
		mexPrintf("Field %s is empty \n", fieldName);
		mexWarnMsgIdAndTxt("SSA:programOptions:StructElementEmpty",
				"The element in the structure is empty,default value will be assigned \n");

		return NULL;
	}
	mexPrintf("The class of field :   %s is : %d\n", fieldName,
			mxGetClassID(fieldPointer));
	if (mxGetClassID(fieldPointer) != classIdExpected)
	{

		mexErrMsgIdAndTxt("SSA:programOptions:inputNotStruct",
				"Given class Id does not match with the expected class id");
	}
	return fieldPointer;
}
