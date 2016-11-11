//
// Created by manish on 01.11.16.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "engine.h"

typedef void (*mexFunction_t)(int nargout, mxArray *pargout [ ], int nargin, const mxArray *pargin[]);

int main(int argc, const char *argv[])

{
    Engine *engine;
    char buffer[1024];
    int i;

    /* matlab must be in the PATH! */
    if (!(engine= engOpen("matlab -nodisplay")))
    {
        fprintf(stderr, "Can't start MATLAB engine\n");
        return -1;
    }
    engOutputBuffer(engine, buffer, 1023);

    /* load the mex file */
    if(argc<2)
    {
        fprintf(stderr, "Error. Give full path to the MEX file as input parameter.\n");
        return -1;
    }
    void *handle = dlopen(argv[1], RTLD_NOW);
    if(handle == NULL)
    {
        fprintf(stderr, "Error loading MEX file: %s\n", strerror(errno));
        return -1;
    }

    /* grab mexFunction handle */
    mexFunction_t mexfunction = (mexFunction_t)dlsym(handle, "mexFunction");
    if(mexfunction == NULL)
    {
        fprintf(stderr, "MEX file does not contain mexFunction\n");
        return -1;
    }

    /* load input data - for convenience do that using MATLAB engine */
    /* NOTE: parameters are MEX-file specific, so one has to modify this*/
    /* to fit particular needs */
    // 'x','parameters', 'options', 'timepoints','timepoints_count'
    engEvalString(engine, "load input.mat");
    mxArray *states= engGetVariable(engine, "x");
    mxArray *parameters= engGetVariable(engine, "parameters");
    mxArray *program_options= engGetVariable(engine, "options");
    mxArray *timepoints= engGetVariable(engine, "timepoints");
    mxArray *timepoints_count= engGetVariable(engine, "timepoints_count");

    // output of mexfuntion
    mxArray *plhs[1] = {0};
    // input to mexfunction
    const mxArray *prhs[5] = {states, parameters, program_options, timepoints, timepoints_count};

    /* execute the mex function */
    mexfunction(1, plhs, 5, prhs);

    /* print the results using MATLAB engine */
    engPutVariable(engine, "result", plhs[0]);
    engEvalString(engine, "result");
    printf("%s\n", buffer);

    /* cleanup */
    mxDestroyArray(plhs[0]);
    engEvalString(engine, "clear all;");
    dlclose(handle);
    engClose(engine);

    return 0;
}
