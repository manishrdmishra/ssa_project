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
#include "program_option_parser_tmp.hpp"

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

/* definition of log levels, lower the value of
 * logging level means higher level of verbosity.
 * INFO - { STATES,PROPENSITIES, REACTION_INDICIES}
 * DEBUG - {T_CURR ,T_NEXT , CHOSEN_PROPENSITY }  + INFO
 * ALL - {RAND_ONE , RAND_TWO } + DEBUG
 * set the logging level for each variable
 * For example,for RAND_ONE the level is set to 0
 * so this variable will be logged when logging flag is
 * set to ALL. If logging flag will be set to DEBUG or
 * INFO then this variable will not be logged.
 * so a variable is logged if its set flag value is higher than
 * the value of logging_flag.
 */
const int log_level_of_var_[ NUM_VARS ] = { 0, 0, 1, 1, 2, 2, 1, 2 };

struct LoggingParameters
{
    std::string panic_file_name_;
    std::string periodic_file_name_;
    long long unsigned num_history_;
    long long unsigned logging_period_;

};

class Logger
{

public:
    Logger(LoggingParameters& logging_parameters)
            :logging_parameters_(logging_parameters)


    {
        openPeriodicFileStream();
    }
    ~Logger()
    {
        closePanicFileStream();
        closePeriodicFileStream();
    }
    inline void setLogLevel(LOGLEVEL log_level)
    {
        level_ = log_level;
    }
    bool shouldBeLogged(VAR var);

    void initializeLoggingFlags();

    void update_logRotation(long long unsigned current_step,
                            const double current_rand_one, double current_rand_two, double current_t_current,
                            double current_t_next, double current_states[], double current_propensities[],
                            double current_chosen_propensity, double current_reaction_index) ;

    void writeOneStep(OUTPUT method, std::ofstream &fstream,
                      long long unsigned current_step,double log_rand_one, double log_rand_two,
                      double log_t_curr, double log_t_next, double log_states[],
                      double log_propensities[], double log_chosen_propensities,
                      double log_reaction_indices);

    void writeLastNSteps(OUTPUT destination, std::ofstream &fstream,
                         long long unsigned current_step );


    inline void openPanicFileStream ()
    {
        openFileStream(logging_parameters_.panic_file_name_, panic_file_stream_);
    }
    inline void openPeriodicFileStream()
    {
        openFileStream(logging_parameters_.periodic_file_name_, periodic_file_stream_);
    }
    inline void closePanicFileStream ()
    {
        closeFileStream(panic_file_stream_);
    }
    inline void closePeriodicFileStream ()
    {
        closeFileStream(periodic_file_stream_);
    }
    inline long long unsigned getLoggingPeriod()
    {
        return logging_parameters_.logging_period_;
    }

    inline long long unsigned getNumOfHistory()
    {
        return logging_parameters_.num_history_;
    }
    inline std::ofstream& getPanicFileStream()
    {
        return panic_file_stream_;
    }

    inline std::ofstream& getPeriodicFileStream ()
    {
        return periodic_file_stream_;
    }

private:
    void openFileStream(std::string file_name, std::ofstream& file_stream);
    void closeFileStream(std::ofstream& file_stream);
    const LoggingParameters& logging_parameters_;

    std::ofstream  panic_file_stream_;
    std::ofstream  periodic_file_stream_ ;


    LOGLEVEL level_;

    /* initialize the logging flag for variables */
    bool logging_flag_of_var_[NUM_VARS];

    /* definition of variables to store the states */
    double log_rand_one_[MAX_HISTORY];
    double log_rand_two_[MAX_HISTORY];
    double log_time_curr_[MAX_HISTORY];
    double log_time_next_[MAX_HISTORY];
    double log_states_ [MAX_HISTORY][SSA_NumStates];
    double log_propensities_ [MAX_HISTORY][SSA_NumReactions];
    double log_chosen_propensity_ [MAX_HISTORY];
    double log_chosen_reaction_index_ [MAX_HISTORY];

};

class LoggerFactory
{

public:
    LoggerFactory(const ProgramOptions* program_options)
    {
        // intantiate and initialize logging parameters
        //logging_parameters_ = new LoggingParameters;
        logging_parameters_.panic_file_name_ = std::string(program_options->panic_file_name());
        logging_parameters_.periodic_file_name_ = std::string(program_options->periodic_file_name());
        logging_parameters_.num_history_ = program_options->num_history();
        logging_parameters_.logging_period_ = program_options->period();
    }
    Logger* create()
    {
        Logger *logger = NULL;
        logger = new Logger(logging_parameters_);
        return logger;
    }

private:
    LoggingParameters logging_parameters_;
};

/* some general purpose macros and function declarations */

// TODO:  This macro should be defined in propensity header file
#define IS_PROPENSITY_NEGATIVE(reaction_id,propensity) if (propensity < 0) { mexPrintf("For reaction : %d the propensity is : %lf\n",reaction_id,propensity); ret_val = -1;}
#endif
