#include "program_option_parser_tmp.hpp"
#include "logger_tmp.hpp"


//Definition of classIDFlags array
mxClassID classIDflags[NUM_OF_FIELDS] =
        {
                mxCHAR_CLASS,
                mxCHAR_CLASS,
                mxUINT64_CLASS,
                mxUINT64_CLASS,
                mxUINT64_CLASS
        };


const ProgramOptions* ProgramOptionsParser::parse()
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

    char* panic_file_name = parseString(field_names.names_,PANIC_FILE);
    char* periodic_file_name = parseString(field_names.names_,PERIODIC_FILE);
    long long unsigned* num_history = parseLong(field_names.names_,NUM_OF_HISTORY);

    if (*num_history > MAX_HISTORY)
    {
        mexWarnMsgIdAndTxt("SSA:programOptions:NUMOFHISTORYTOOBIG",
                           "The value provided of number of history is too big, changing it to maximum allowed value\n");
        *num_history = MAX_HISTORY;
    }

    long long unsigned* period = parseLong(field_names.names_,PERIOD);
    long long unsigned* num_of_threads = parseLong(field_names.names_,NUM_OF_THREAD);
    ProgramOptions* program_options = new ProgramOptions(panic_file_name,
                                                         periodic_file_name,
                                                         num_history,
                                                         period,
                                                         num_of_threads);
    mexPrintf("parsing done!\n");
    return program_options;

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
