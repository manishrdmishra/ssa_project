#ifndef LOGGER_HPP_
#define LOGGER_HPP_

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
enum OUTPUT {
    STD_OUTPUT = 0, FILE_OUTPUT
};

/* Defines the log level */
enum LOGLEVEL {
    ALL = 0, DEBUG, INFO, OFF
};

struct LoggingParameters
{
    std::string panic_file_name_;
    std::string periodic_file_name_;
    long long unsigned num_history_;
    long long unsigned logging_period_;

};

class Logger {

public:
    Logger(LoggingParameters& logging_parameters)
    :logging_parameters_(logging_parameters)
    {
    }
    inline void setLogLevel(LOGLEVEL log_level)
    {
        level_ = log_level;
    }
    bool shouldBeLogged(VAR var);

    void initializeLoggingFlags();
 inline void initializeLoggingLevelOfVar()
    {
        log_level_of_var_[RAND_ONE] = 0;
   log_level_of_var_[RAND_TWO] = 0;
   log_level_of_var_[T_CURR] = 1;
   log_level_of_var_[T_NEXT] = 1;
   log_level_of_var_[STATES] = 2;
   log_level_of_var_[PROPENSITIES] = 2;
   log_level_of_var_[CHOSEN_PROPENSITY] = 1;
   log_level_of_var_[REACTION_INDEX] = 2;

    }
    void update_logRotation(long long unsigned current_step,
                            double current_rand_one, double current_rand_two, double current_t_current,
                            double current_t_next, double current_states[], double current_propensities[],
                            double current_chosen_propensity, double current_reaction_index);

    void writeOneStep(OUTPUT method, std::ofstream &fstream,
                      long long unsigned current_step,double log_rand_one, double log_rand_two,
                      double log_t_curr, double log_t_next, double log_states[],
                      double log_propensities[], double log_choosen_propensities,
                      double log_reaction_indices);

    void writeLastNSteps(OUTPUT destination, std::ofstream &fstream,
                         long long unsigned currentHistoryStep );

//void writePeriodicLog(OUTPUT method, std::ofstream& fstream,
//		long long unsigned current_step, LOGLEVEL level,
//		bool* logging_flag_of_var, double log_rand_one, double log_rand_two,
//		double log_t_curr, double log_t_next, double log_states[],
//		double log_propensities[], double log_choosen_propensities,
//		double log_reaction_indices);
    void openOutputStream(std::string file_name, std::ofstream &fstream);

    void closeOutputStream(std::ofstream &fstream);
    //void setPanicFileName(const std::string file_name);
    //void setPeriodicFileName(const std::string file_name);
    //void setNumOfHistory(const )

private:


    LoggingParameters& logging_parameters_;

    //std::cout<<"logging enabled..\n"<<std::endl;

    /* definition of log levels, lower the value of
     * logging level means higher level of verbosity.
     *
     * INFO - { STATES,PROPENSITIES, REACTION_INDICIES}
     * DEBUG - {T_CURR ,T_NEXT , CHOOSEN_PROPENSITY }  + INFO
     * ALL - {RAND_ONE , RAND_TWO } + DEBUG
     * set the logging level for each variable
     * For example,for RAND_ONE the level is set to 0
     * so this variable will be logged when logging flag is
     * set to ALL. If logging flag will be set to DEBUG or
     * INFO then this variable will not be logged.
     * so a varible is logged if its set flag value is higher than
     * the value of logging_flag.
     */
    int log_level_of_var_[ NUM_VARS ];
     /*=
            {
                    0  , // RAND_ONE
                    0  , // RAND_ONE
                    1  , // T_CURR
                    1  , // T_NEXT
                    2  , // STATES
                    2  , // PROPENSITIES
                    1  , // CHOSEN_PROPENSITY
                    2  , // REACTION_INDEX
            };
*/

    /*log_level_of_var[RAND_ONE] = 0;
    log_level_of_var[RAND_TWO] = 0;
    log_level_of_var[T_CURR] = 1;
    log_level_of_var[T_NEXT] = 1;
    log_level_of_var[STATES] = 2;
    log_level_of_var[PROPENSITIES] = 2;
    log_level_of_var[CHOSEN_PROPENSITY] = 1;
    log_level_of_var[REACTION_INDEX] = 2;
*/
    /* Panic log file name */
  //  std::string panic_file_name(panicFileName);
   // std::ofstream panic_fstream;

    /* periodic log file */
    //std::string periodic_file_name(periodicFileName);
    //std::ofstream periodic_fstream;
    //openOutputStream(periodic_file_name, periodic_fstream);

    LOGLEVEL level_;

    /* initialize the logging flag for variables */
    bool logging_flag_of_var_[NUM_VARS];
    //= {false, false, false, false, false, false, false, false};

    /* definition of variables to store the states */
    double log_rand_one_[MAX_HISTORY];
    double log_rand_two_[MAX_HISTORY];
    double log_time_curr_[MAX_HISTORY];
    double log_time_next_[MAX_HISTORY];
    double log_states_ [MAX_HISTORY][SSA_NumStates];
    double log_propensities_ [MAX_HISTORY][SSA_NumReactions];
    double log_chosen_propensity_ [MAX_HISTORY];
    double log_chosen_reaction_index_ [MAX_HISTORY];

    long long unsigned history_counts_ ;
};
mxArray* getFieldPointer(const mxArray *struct_array, int index,
                         const char* fieldName, mxClassID classIdExpected);

//void ssaCalloc(double **fieldPtr, mwSize n);
//void ssaCalloc(double ***fieldPtr, mwSize n , mwSize m );

#define CHECK_NOTNEG(reaction_id,propensity) if (propensity < 0) { mexPrintf("For reaction : %d the propensity is : %lf\n",reaction_id,propensity);return -1;}
#endif
