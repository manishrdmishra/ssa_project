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

/* Pre assign the class of each element present in the structure */
mxClassID classIDflags[NUM_OF_FIELDS] =
        {
                mxCHAR_CLASS,
                mxCHAR_CLASS,
                mxUINT64_CLASS,
                mxUINT64_CLASS,
                mxUINT64_CLASS
        };

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
 *  num_of_threads - Number of threads used to execute parallel sections
 *
 * *****************************************************************/


class ProgramOptionsParser
{
public:
    ProgramOptionsParser(const mxArray* options);
    void parse();
    char* periodic_file_name() const;
    char* panic_file_name() const;
    long long unsigned num_history()const;
    long long unsigned period() const;
    long long unsigned num_threads() const;
    ~ProgramOptionsParser();
    mxArray* getFieldPointer(const char* fieldName, mxClassID classIdExpected);
private:
    const mxArray* options_;
    char* panic_file_name_;
    char* periodic_file_name_;
    long long unsigned* num_history_;
    long long unsigned* period_;
    long long unsigned* num_of_threads_;
    long long unsigned* parseLong(const char** field_names, PROGRAM_OPTIONS option);
    char* parseString(const char** names, PROGRAM_OPTIONS option);

};

ProgramOptionsParser::ProgramOptionsParser(const mxArray *options)
        :options_(options),
         panic_file_name_(NULL),
         periodic_file_name_(NULL),
         num_history_(NULL),
         period_(NULL),
         num_of_threads_(NULL)
{

}
ProgramOptionsParser::~ProgramOptionsParser()
{

}


inline char* ProgramOptionsParser::panic_file_name() const
{
    return panic_file_name_;
}
inline char* ProgramOptionsParser::periodic_file_name() const
{
    return periodic_file_name_;
}
inline long long unsigned ProgramOptionsParser::num_history() const
{
    return *num_history_;
}
inline long long unsigned ProgramOptionsParser::period() const
{
    return *period_;
}
inline long long unsigned ProgramOptionsParser::num_threads() const
{
    return *num_of_threads_;
}
void ProgramOptionsParser::parse()
{
    /* get number of elements in structure */
    int num_fields = mxGetNumberOfFields(options_);
    //mexPrintf("Number of fields provided in structure %d \n", num_fields);
    if (num_fields != NUM_OF_FIELDS)
    {
        mexWarnMsgIdAndTxt("SSA:programOptions:NumOfStructElementMismatch",
                           "The expected number of elements in structure does not match with the provided\n");
        //mexPrintf("Expected vs Provided : %d  %d\n", NUM_OF_FIELDS, num_fields);
    }

    //mexPrintf("Number of elements in structure %d \n", num_of_elements_in_structure);
    FieldNames field_names(num_fields,options_);

    panic_file_name_ = parseString(field_names.names_,PANIC_FILE);
    periodic_file_name_ = parseString(field_names.names_,PERIODIC_FILE);
    num_history_ = parseLong(field_names.names_,NUM_OF_HISTORY);
    period_ = parseLong(field_names.names_,PERIOD);
    num_of_threads_ = parseLong(field_names.names_,NUM_OF_THREAD);
    mexPrintf("parsing done!\n");

// memory allocated in FieldNames will be free here
}
long long unsigned* ProgramOptionsParser::parseLong(const char** names, PROGRAM_OPTIONS option_index)
{
/* get the filed pointer*/
    mxArray *field_pointer= getFieldPointer(names[option_index], classIDflags[option_index]);
    if (field_pointer != NULL)
    {

       return (long long unsigned*) mxGetData(field_pointer);
    }
}
char* ProgramOptionsParser::parseString(const char** names, PROGRAM_OPTIONS option_index)
{
    /* get the filed pointer*/
    mxArray *string_field_pointer = getFieldPointer(names[option_index], classIDflags[option_index]);
    char* string_field = NULL;
    if (string_field_pointer != NULL)
    {

        string_field = mxArrayToString(string_field_pointer);
        mexPrintf("string field : %s \n", string_field);
    }
    return string_field;
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
mxArray* ProgramOptionsParser::getFieldPointer(const char* field_name, mxClassID class_id_expected)
{
    mxArray *field_pointer = NULL;
    //mexPrintf("executing getFieldPointer..");
    field_pointer = mxGetField(options_, 0 , field_name);
    if (field_pointer == NULL || mxIsEmpty(field_pointer)) {
        //mexPrintf("Field %s is empty \n", fieldName);
        mexWarnMsgIdAndTxt("SSA:programOptions:StructElementEmpty",
                           "The element in the structure is empty,default value will be assigned \n");

        return NULL;
    }
//	mexPrintf("The class of field :   %s is : %d\n", fieldName,
//			mxGetClassID(field_pointer));
    if (mxGetClassID(field_pointer) != class_id_expected) {

        mexErrMsgIdAndTxt("SSA:programOptions:inputNotStruct",
                          "Given class Id does not match with the expected class id");
//        mexPrintf("The expected class of field :   %s is : %d\n", fieldName,
//			classIdExpected);
    }
    return field_pointer;
}

#endif //DR_SSASIM_PROGRAM_OPTION_PARSER_HPP
