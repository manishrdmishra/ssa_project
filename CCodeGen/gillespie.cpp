//
// Created by manish on 13.10.16.
//

#include "gillespie_tmp.hpp"
#include "gillespie.hpp"

void Gillespie::initializeSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{
    std::cout<<"initializing the simulation\n";
    itime_ = 0;
    /* write initial conditions to output */
    for ( int i  = 0; i < SSA_NumStates ;  i++)
    {

        simulation_parameters_out.timecourse_[itime_ * SSA_NumStates + i] = simulation_parameters_in.states_[i];
    }

    itime_++;
    time_next_ = simulation_parameters_in.timepoints_[itime_];

    /* start iteration of simulation */
    time_curr_ = simulation_parameters_in.timepoints_[0];
    time_next_ = simulation_parameters_in.timepoints_[itime_];

}
void Gillespie::writeStatesToOutputOnTimeOut(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{
    // if time > time_out then write next data point to output
    int index = 0;
    while ( time_curr_ >= time_next_ && itime_ < simulation_parameters_in.timepoints_count_)
    {
        index = itime_ * SSA_NumStates;

        for ( int i = 0 ; i < SSA_NumStates ; i ++)
        {
            simulation_parameters_out.timecourse_[index + i ] = simulation_parameters_in.states_[i];
        }
        itime_++;
        time_next_ = simulation_parameters_in.timepoints_[itime_];

    }
}

GillespieBasic::GillespieBasic(Logger& logger)
        :logger_(logger)
{

}
GillespieBasic::~GillespieBasic()
{
    //free the memory and close the open streams
}

void GillespieBasic::runSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{

    // initialize simulation
    std::cout<<"initializing the simulation\n";
    initializeSimulation(simulation_parameters_in, simulation_parameters_out);

    // declaration of some local variables
    double chosen_propensity = 0;
    double rand_one = 0, rand_two = 0;
    int return_val = 0;
    double temp = 0;

    while ( time_curr_ < simulation_parameters_in.timepoints_[simulation_parameters_in.timepoints_count_ - 1])
    {
        //std::cout<<"looping\n";
        // generate two random numbers
        rand_one = generateRandomNumber();
        rand_two = generateRandomNumber();
        // calculate cumulative propensity
       // std::cout<<"calculating the propensity\n";

//        std::cout<<"print states\n";
//
//        for( int i = 0 ; i<4 ; i ++)
//        {
//            std::cout <<simulation_parameters_in.states_[i]<<"\n";
//        }

   // std::cout<<"time points : "<<simulation_parameters_in.timepoints_count_<<std::endl;
   // std::cout<<"parameters : "<<simulation_parameters_in.parameters_[0]<<std::endl;


        return_val = calculateCumulativePropensity(cumulative_propensity_,simulation_parameters_in.states_ ,simulation_parameters_in.parameters_);
//        std::cout<<"print propensity\n";
//
//        for( int i = 0 ; i<7 ; i ++)
//        {
//            std::cout <<cumulative_propensity_[i]<<"\n";
//        }
        if (return_val == -1 )
        {
            // if logging enabled log the last n steps in file
            mexErrMsgIdAndTxt("SSA:InvalidPropensity",
                              "Propensity can not be negative");
        }
        // draw exponential random variable with parameter a0 = cumulative_propensity_[SSA_NumReactions - 1]

        temp = 1 / cumulative_propensity_[SSA_NumReactions - 1] * log ( 1 / rand_one) ;
        // check if temp is not infinity
        // if temp is inifinity then stop the simulation
        // and log the last n  steps of the simulation, if logging is enabled

        time_curr_ = time_curr_ + temp ;
        // if time has been out then write the states to output
        writeStatesToOutputOnTimeOut(simulation_parameters_in, simulation_parameters_out);

        // choose reaction index
        chosen_propensity = rand_two * cumulative_propensity_[SSA_NumReactions - 1 ];

        //start searching form first reaction
        reaction_index_ = 1;

        // scan all the propensities to find the reaction index
        for (int i = 1 ; cumulative_propensity_[ i - 1 ] <= chosen_propensity; i++)
        {
            reaction_index_ = i + 1;
        }
        std::cout<<"chosen reaction index : "<<reaction_index_<<std::endl;

        // update the  states according to reaction index
        updateSsaState(simulation_parameters_in.states_,reaction_index_);

#ifdef  LOGGING
        *//* this call store the parameters of simulation which can used to print
		 * at later stage in case of any error
		 *//*

		//std::cout<<"updating logs...\n"<<std::endl;
		logger_.update_logRotation(history_counts_,rand_one,
				rand_two,time_curr_,time_next_,
				simulation_parameters_in.states_,
				cumulative_propensity_,
				chosen_propensity ,reaction_index_);

		history_counts_ = history_counts_ + 1;
		if (historyCounts > *numHistory)
		{
			historyCounts = 0;
		}

		*//* write the current state of the system to log file *//*
		if (globalCounter % *period == 0)
		{

			//mexPrintf("printing logs..");
			writeOneStep(FILE_OUTPUT,periodic_fstream,globalCounter, level, logging_flag_of_var, rand1,
					rand2, tCurr,tNext,
					xCurr,cumProps,
					chosenProp, reactionIndex);

		}
#endif

        global_counter_ = global_counter_ + 1;

    }



}
