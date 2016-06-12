#include "mex.h"
#include "DRTB_modeldefHeader_tmp.hpp"
#include <cmath>
#include <algorithm>
#include <omp.h> 
#include <iostream>
#include <glog/logging.h>



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
    int     numTimepts;

    
    /* Load input values from prhs */    
    xCurr      = mxGetPr(prhs[0]);
    parameters = mxGetPr(prhs[1]);
    timepoints = mxGetPr(prhs[2]);
    numTimepts = (int)mxGetScalar(prhs[3]);
    
    
    /* Declare IMs*/
    double cumProps[SSA_NumReactions];
    int    reactionIndex;
    int    iTime;
    double tCurr;
    double tNext;

    /* Declare Outputs*/
    double* timecourse;
    
    /* Create Outputs I */
    plhs[0]    = mxCreateDoubleMatrix(SSA_NumStates*numTimepts,1,mxREAL);
    timecourse = mxGetPr(plhs[0]);

    
    /* Write initial conditions to output */
    iTime = 0;
	for(int i = 0;i<SSA_NumStates;i++)
    {
        timecourse[iTime*SSA_NumStates + i] = xCurr[i];
    }
    iTime++;
    tNext = timepoints[iTime];
    
    
    /* Start iteration*/
    tCurr = timepoints[0];
    tNext = timepoints[iTime];
    int everythingCounts = 0;
   // LOG(INFO) << "Log using default file";
    while(tCurr < timepoints[numTimepts-1])
    {
        // Debugging info - massive performance decrease
        double rand1 = std::max(1.0,(double)rand())/(double)RAND_MAX;
        double rand2 = std::max(1.0,(double)rand())/(double)RAND_MAX;

        /* Calculate cumulative propensities in one step*/
        calculateCumProps(cumProps, xCurr, parameters);
        /* the propensity of each reaction can be printed out here*/

        /* Sample reaction time*/
        tCurr = tCurr + 1/cumProps[SSA_NumReactions-1]*log(1/rand1);
        
        // Debugging information
        //if(everythingCounts >= 50000)
        //{
        //  mexPrintf("CPP main calculation: tnow %.8fs | tnext %.8fs | 1/prop %.16f | log(1/prop) %.4f |rand %.4f | dt %.16f\n", tCurr, tNext,1/cumProps[SSA_NumReactions-1], log10(1/cumProps[SSA_NumReactions-1]),log(1/rand1),1/cumProps[SSA_NumReactions-1]*log(1/rand1));
        //  everythingCounts = 0;
        //}

        /* If time > time out, write next datapoint to output*/
        while(tCurr >= tNext && iTime < numTimepts)
        {
            // Debugging information
           // mexPrintf("This is bound to be good: tnow %.8fs | tnext %.8fs | 1/prop %.16f | log(1/prop) %.4f |rand %.4f | dt %.16f\n", tCurr, tNext,1/cumProps[SSA_NumReactions-1], log10(1/cumProps[SSA_NumReactions-1]),log(1/rand1),1/cumProps[SSA_NumReactions-1]*log(1/rand1));            
//             omp_set_dynamic(1);
//             #pragma omp parallel
//              omp_set_num_threads(4);
//             #pragma omp parallel for
           // mexPrintf("Current count of states: ");
            for(int i = 0;i<SSA_NumStates;i++){
                //cIndex  =  iTime *  SSA_NumStates this will save the repeated calcuation 
                
                timecourse[iTime*SSA_NumStates + i] = xCurr[i];
             //   mexPrintf(" %d",xCurr[i]);
            }
            //mexPrintf("\n");
            iTime++;
            tNext = timepoints[iTime];
        }

        /* Sample reaction index*/
        double chosenProp = rand2 * cumProps[SSA_NumReactions-1];
        reactionIndex = 1;
        for(int i = 1; cumProps[i-1] <= chosenProp; i++)
            reactionIndex = i+1;

        /* Update xCurr */ 
        updateState(xCurr, reactionIndex);
        everythingCounts = everythingCounts + 1;
    }
}

