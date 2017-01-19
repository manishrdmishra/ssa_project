//
// Created by manish on 13.10.16.
//
#include <algorithm>
#include <iterator>
#include "gillespie_tmp.hpp"


void Gillespie::initializeSimulation(SimulationParametersIn& simulation_parameters_in,
                                     SimulationParametersOut& simulation_parameters_out)
{
    itime_ = 0;
    // copy initial state vector
    std::copy(simulation_parameters_in.states_,
              simulation_parameters_in.states_+ SSA_NumStates,
              simulation_parameters_out.timecourse_ );

    // initialize the current time of simulation
    time_curr_ = simulation_parameters_in.time_points_[0];

    // initialize the next time of simulation
    ++itime_;
    time_next_ = simulation_parameters_in.time_points_[itime_];

}
void Gillespie::copyStateVectorTillTimeOut(SimulationParametersIn& simulation_parameters_in,
                                             SimulationParametersOut& simulation_parameters_out)
{
    // if time > time_out then write next data point to output
    for(int index = itime_ * SSA_NumStates;
        ( time_curr_ >= time_next_) &&
        (itime_ < simulation_parameters_in.time_points_count_);)
    {
        std::copy(simulation_parameters_in.states_,
                  simulation_parameters_in.states_ + SSA_NumStates,
                  simulation_parameters_out.timecourse_ + index);

        ++itime_;
        time_next_ = simulation_parameters_in.time_points_[itime_];
    }
}

void GillespieBasic::runSimulation(SimulationParametersIn& simulation_parameters_in,
                                   SimulationParametersOut& simulation_parameters_out)
{

    // initialize simulation
    initializeSimulation(simulation_parameters_in, simulation_parameters_out);

    // initialize local variables
    long long unsigned history_counts = 0;
    long long unsigned global_counter = 0;
    double chosen_propensity = 0;
    double rand_one = 0, rand_two = 0;
    int return_val = 0;

    while ( time_curr_ < simulation_parameters_in.time_points_[simulation_parameters_in.time_points_count_ - 1])
    {
        // generate two random numbers
        rand_one = generateRandomNumber();
        rand_two = generateRandomNumber();

        // calculate cumulative propensity
        return_val = calculateCumulativePropensity(cumulative_propensity_,
                                                   simulation_parameters_in.states_ ,
                                                   simulation_parameters_in.parameters_);
        if (return_val == -1 )
        {
            // propensity is negative
            mexErrMsgIdAndTxt("SSA:InvalidPropensity",
                              "Propensity can not be negative");
#ifdef LOGGING
            // if logging enabled log the last n steps in file
            logger_->openPanicFileStream();
			logger_->writeLastNSteps(FILE_OUTPUT,logger_->getPanicFileStream(), history_counts);

#endif
        }

        // draw exponential random variable with parameter a0 = cumulative_propensity_[SSA_NumReactions - 1]
        time_curr_ += calculateExponentialRandomValue(rand_one);
        // if time has been out then write the states to output
        copyStateVectorTillTimeOut(simulation_parameters_in, simulation_parameters_out);

        // choose a propensity
        chosen_propensity = chooseRandomPropensity(rand_two);
        reaction_index_ = findNextReactionIndex(chosen_propensity);

        // update the  states according to reaction index
        updateSsaState(simulation_parameters_in.states_,reaction_index_);

#ifdef  LOGGING

        // this call store the parameters of simulation which can used to print
		//  at later stage in case of any error
		logger_->update_logRotation(history_counts,rand_one,
				                    rand_two,time_curr_,time_next_,
				                    simulation_parameters_in.states_,
				                    cumulative_propensity_,
				                    chosen_propensity ,reaction_index_);

		if (history_counts > logger_->getNumOfHistory())
		{
			history_counts = 0;
		}

		++history_counts;

		// write the current state of the system to log file
		if (global_counter % logger_->getLoggingPeriod() == 0)
		{

			//mexPrintf("printing logs..");
			logger_->writeOneStep(FILE_OUTPUT,logger_->getPeriodicFileStream(),
			                     global_counter, rand_one, rand_two, time_curr_,
			                     time_next_, simulation_parameters_in.states_,
			                     cumulative_propensity_, chosen_propensity, reaction_index_);

		}
#endif

        ++global_counter;

    }

}

inline int GillespieBasic::findNextReactionIndex(double chosen_propensity)
{
    //find first reaction for which propensity is greater than
    // chosen propensity
    double* it = std::find_if(cumulative_propensity_,
                              cumulative_propensity_ + SSA_NumReactions,
                              custom_greater(chosen_propensity));
    // add one, as reactions are 1-indexed
    return (std::distance(cumulative_propensity_,it) + 1);

}