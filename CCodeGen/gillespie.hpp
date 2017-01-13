//
// Created by manish on 11.10.16.
//

#ifndef DR_SSASIM_GELLESPIE_H
#define DR_SSASIM_GELLESPIE_H

#include <cmath>
#include <algorithm>
#include <limits>

#include "DRTB_modeldefHeader_tmp.hpp"
#include "logger_tmp.hpp"

struct SimulationParametersIn
{

    double* states_ ;
    double* parameters_;
    double* time_points_;
    int time_points_count_;
};

struct SimulationParametersOut
{

    double* timecourse_ ;
};

class Gillespie
{
public:


    Gillespie ()
    {

    };
    virtual ~Gillespie ()
    {

    };
    virtual void runSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out)
    {

    }

protected:

    void initializeSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out);
    inline double generateRandomNumber ()
    {
        return ( std::max(1.0, ( double ) rand()) / (double) RAND_MAX );
    };
    int calculateCumulativePropensity(double* cumulative_propensity, double* states, double* parameters)
    {
        return ( calculateCumProps(cumulative_propensity,states, parameters));
    }
    void updateSsaState(double* states, const int reaction_index)
    {
        updateState( states, reaction_index);
    }
    void writeStatesToOutputOnTimeOut(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out);
    double cumulative_propensity_[SSA_NumReactions];
    int reaction_index_;
    int itime_;
    double time_curr_;
    double time_next_;
};

class GillespieBasic : public Gillespie
{
public:

    GillespieBasic(Logger* logger);
    ~GillespieBasic();
    void runSimulation(SimulationParametersIn& simulation_parameters_in, SimulationParametersOut& simulation_parameters_out);
private:
    Logger* logger_;
};


#endif //DR_SSASIM_GELLESPIE_H
