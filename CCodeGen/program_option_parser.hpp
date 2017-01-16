#include "mex.h"
#include <string>
#include <cstring>

#ifndef DR_SSASIM_PROGRAM_OPTION_PARSER_HPP
#define DR_SSASIM_PROGRAM_OPTION_PARSER_HPP



/****************************************************************
 * NUM_OF_FIELDS - This is the current number of fields present
 * in the structure program_options.
 *****************************************************************/

#define NUM_OF_FIELDS 5

/* index for program_options structure elements
 * ie. the first field is fixed for the name of panic file
 * , second field is fixed for periodic file name and so on.
 */

enum PROGRAM_OPTIONS
{
    PANIC_FILE = 0,
    PERIODIC_FILE,
    NUM_OF_HISTORY,
    PERIOD,
    NUM_OF_THREAD
};

/* Pre assign the class of each element present in the structure,
 * see the source file for definition
 */
extern mxClassID classIDflags[NUM_OF_FIELDS];

struct FieldNames
{
    FieldNames(size_t num_fields,const mxArray* options)
    {

        names_= (const char**) mxCalloc(num_fields, sizeof(char*));
        /* get field name pointers */
        for (int i = 0; i != num_fields; ++i)
        {
            names_[i] = mxGetFieldNameByNumber(options, i);

        }
    }

    ~FieldNames()
    {
        /* free the memory */
        mxFree((void *) names_);
    }
    const char** names_;
};




/****************************************************************
 * mexFunction receives a structure in prhs[4].
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
 *  num_of_threads - Number of threads used to execute parallel sections
 *
 * *****************************************************************/


class ProgramOptions
{
public:
    ProgramOptions(char* panic_file_name,
                   char* periodic_file_name,
                   long long unsigned* num_history,
                   long long unsigned* period,
                   long long unsigned* num_of_threads)
            : panic_file_name_(panic_file_name),
              periodic_file_name_(periodic_file_name),
              num_history_(num_history),
              period_(period),
              num_of_threads_(num_of_threads)
    {

    }
    char* periodic_file_name() const
    {
        return periodic_file_name_;
    }
    char* panic_file_name() const
    {
        return panic_file_name_;
    }
    long long unsigned num_history() const
    {
        return *num_history_;
    }
    long long unsigned period() const
    {
        return *period_;
    }
    long long unsigned num_threads() const
    {
        return *num_of_threads_;
    }
    ~ProgramOptions()
    {

    }
private:
    char* panic_file_name_;
    char* periodic_file_name_;
    long long unsigned* num_history_;
    long long unsigned* period_;
    long long unsigned* num_of_threads_;

};

class ProgramOptionsParser
{
public:
    ProgramOptionsParser(const mxArray* options)
    :options_(options)
    {

    }
    const ProgramOptions* parse();
    ~ProgramOptionsParser()
    {

    }
    mxArray* getFieldPointer(const char* fieldName, mxClassID classIdExpected);
private:
    const mxArray* options_;
    long long unsigned* parseLong(const char** field_names, PROGRAM_OPTIONS option);
    char* parseString(const char** names, PROGRAM_OPTIONS option);


};

#endif //DR_SSASIM_PROGRAM_OPTION_PARSER_HPP
