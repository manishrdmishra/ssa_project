//
// Created by manish on 13.10.16.
//

#include "gillespie_tmp.hpp"

void Gillespie::initializeSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{
    //std::cout<<"initializing the simulation\n";
    itime_ = 0;
    /* write initial conditions to output */
    for ( int i  = 0; i < SSA_NumStates ;  i++)
    {

        simulation_parameters_out.timecourse_[itime_ * SSA_NumStates + i] = simulation_parameters_in.states_[i];
    }

    itime_++;
    time_next_ = simulation_parameters_in.time_points_[itime_];

    /* start iteration of simulation */
    time_curr_ = simulation_parameters_in.time_points_[0];
    time_next_ = simulation_parameters_in.time_points_[itime_];

}
void Gillespie::writeStatesToOutputOnTimeOut(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{
    // if time > time_out then write next data point to output
    int index = 0;
    while ( time_curr_ >= time_next_ && itime_ < simulation_parameters_in.time_points_count_)
    {
        index = itime_ * SSA_NumStates;

        for ( int i = 0 ; i < SSA_NumStates ; i ++)
        {
            simulation_parameters_out.timecourse_[index + i ] = simulation_parameters_in.states_[i];
        }
        itime_++;
        time_next_ = simulation_parameters_in.time_points_[itime_];

    }
}

void GillespieBasic::runSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{

    // initialize simulation
    //std::cout<<"initializing the simulation\n";
    initializeSimulation(simulation_parameters_in, simulation_parameters_out);

    // declaration of some local variables
    long long unsigned history_counts = 0;
    long long unsigned global_counter = 0;
    double chosen_propensity = 0;
    double rand_one = 0, rand_two = 0;
    int return_val = 0;
    double temp = 0;

    while ( time_curr_ < simulation_parameters_in.time_points_[simulation_parameters_in.time_points_count_ - 1])
    {
        //std::cout<<"looping\n";
        // generate two random numbers
        rand_one = generateRandomNumber();
        rand_two = generateRandomNumber();

        // calculate cumulative propensity
        // std::cout<<"calculating the propensity\n";
        return_val = calculateCumulativePropensity(cumulative_propensity_,simulation_parameters_in.states_ ,simulation_parameters_in.parameters_);
        if (return_val == -1 )
        {
            // if logging enabled log the last n steps in file
            mexErrMsgIdAndTxt("SSA:InvalidPropensity",
                              "Propensity can not be negative");
#ifdef LOGGING

            logger_->openPanicFileStream();
			logger_->writeLastNSteps(FILE_OUTPUT,logger_->getPanicFileStream(), history_counts);

#endif
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
        //std::cout<<"chosen reaction index : "<<reaction_index_<<std::endl;

        // update the  states according to reaction index
        updateSsaState(simulation_parameters_in.states_,reaction_index_);

#ifdef  LOGGING
        /* this call store the parameters of simulation which can used to print
		 * at later stage in case of any error
		 */

		//std::cout<<"updating logs...\n"<<std::endl;
		logger_->update_logRotation(history_counts,rand_one,
				rand_two,time_curr_,time_next_,
				simulation_parameters_in.states_,
				cumulative_propensity_,
				chosen_propensity ,reaction_index_);

		if (history_counts > logger_->getNumOfHistory())
		{
			history_counts = 0;
		}

		history_counts = history_counts + 1;

		/* write the current state of the system to log file */
		if (global_counter % logger_->getLoggingPeriod() == 0)
		{

			//mexPrintf("printing logs..");
			logger_->writeOneStep(FILE_OUTPUT,logger_->getPeriodicFileStream(),global_counter, rand_one,
					rand_two, time_curr_,time_next_,
					simulation_parameters_in.states_,cumulative_propensity_,
					chosen_propensity, reaction_index_);

		}
#endif

        global_counter = global_counter + 1;

    }



}
