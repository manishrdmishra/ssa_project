//
// Created by manish on 11.10.16.
//

#ifndef DR_SSASIM_GELLESPIE_H
#define DR_SSASIM_GELLESPIE_H

#include <cmath>
#include <algorithm>
#include <limits>

#include "DRTB_modeldefHeader_tmp.hpp"

struct SimulationParametersIn
{

    double* states_ ;
    double* paramters_;
    double* timepoints_;
    int timepoints_count_;
};

struct SimulationParametersOut
{

    double* timecourse_ ;
};

class Gillespie
{
public:
    virtual Gillespie (Logger& logger);
    virtual ~Gillespie ();
    void initializeSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out);
    inline double generateRandomNumber ()
    {
        return ( std::max(1.0, ( double ) rand() / (double) RAND_MAX) );
    };
    inline int calculateCumulativePropensity(double* cumulative_propensity, double* states, double* parameters)
    {
        return ( calculateCumProps(cumulative_propensity,states, parameters));
    }
    inline void updateState(double* states, const int reaction_index)
    {
        updateState( states, reaction_index)
    }
    void writeStatesToOutputOnTimeOut(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out);
    virtual void runSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out) = 0;

protected:
    double cumulative_propensity_[SSA_NumReactions];
    int reaction_index_;
    int itime_;
    double tcurr_;
    double tnext_;
    Logger logger_;
};
void Gillespie::initializeSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{
    itime_ = 0;
    /* write initial conditions to output */
    for ( int i  = 0; i < SSA_NumStates ;  i++)
    {

        simulation_parameters_out.timecourse_[itime_ * SSA_NumStates + i] = simulation_parameters_in.states_[i];
    }

    itime_++;
    tnext_ = simulation_parameters_in.timepoints_[itime_];

    /* start iteration of simulation */
    tcurr_ = simulation_parameters_in.timepoints_[0];
    tnext_ = simulation_parameters_in.timepoints_[itime_];

}
void Gillespie::writeStatesToOutputOnTimeOut(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
{
    // if time > time_out then write next data point to output
    int index = 0;
    while ( tcurr_ >= tnext_ && itime_ < simulation_parameters_in.timepoints_count_)
    {
        index = itime_ * SSA_NumStates;

        for ( int i = 0 ; i < SSA_NumStates ; i ++)
        {
            simulation_parameters_out.timecourse_[index + i ] = simulation_parameters_in.states_[i];
        }
        itime_++;
        tnext_ = simulation_parameters_in.timepoints_[itime_];

    }
}
class GillespieBasic : public Gillespie
{
public:

    GillespieBasic(Logger& logger);
    ~GillespieBasic();
    void runSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out);

};

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

    // declaration of some local variables
    double chosen_propensity = 0;
    double rand_one = 0, rand_two = 0;
    int return_val = 0;
    double temp = 0;


    while ( tcurr_ < simulation_parameters_in.timepoints_[simulation_parameters_in.timepoints_count_ - 1])
    {
        // generate two random numbers
        rand_one = generateRandomNumber();
        rand_two = generateRandomNumber();
        // calculate cumulative propensity
        return_val = calculateCumulativePropensity(cumulative_propensity_,simulation_parameters_in.states_ ,simulation_parameters_in.paramters_);
        if (return_val == -1 )
        {
            // if logging enabled log the last n steps in file
            mexErrMsgIdAndTxt("SSA:InvalidPropensity",
                              "Propensity can not be negative");
        }
        // draw exponential random variable with parameter a0 = cumulative_propensity_[SSA_NumReactions - 1]

        double temp = 1 / cumulative_propensity_[SSA_NumReactions - 1] * std::log ( 1 / rand_one) ;
        // check if temp is not infinity
        // if temp is inifinity then stop the simulation
        // and log the last n  steps of the simulation, if logging is enabled

        tcurr_ = tcurr_ + temp ;
        // if time has been out then write the states to output
        writeStatesToOutputOnTimeOut(simulation_parameters_in, simulation_parameters_out);

        // choose reaction index
        chosen_propensity = rand_two * cumulative_propensity_[SSA_NumREactions - 1 ];

        //start searching form first reaction
        reaction_index_ = 1;

        // scan all the propensities to find the reaction index
        for (int i = 0 ; cumulative_propensity_[ i - 1 ] <= choosen_propensity; i++)
        {
            reaction_index_ = i + 1;
        }

        // update the  states according to reaction index
        updateState(simulation_parameters_in.states_,reaction_index_);

    }


}

#endif //DR_SSASIM_GELLESPIE_H
