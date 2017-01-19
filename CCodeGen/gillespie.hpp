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

// A custom greater functor
class custom_greater
{
public:
    custom_greater(double chosen_propensity)
            :chosen_propensity_(chosen_propensity)
    {

    }

    bool operator()(double propensity)
    {
        return propensity > chosen_propensity_;
    }

private:
    double chosen_propensity_;
};

class Gillespie
{

public:
enum TYPE
    {
       BASIC = 0
    };

    Gillespie ()
    {

    };
    virtual ~Gillespie ()
    {

    };
    virtual void runSimulation(SimulationParametersIn& simulation_parameters_in,
                               SimulationParametersOut& simulation_parameters_out)
    {

    }

protected:

   void initializeSimulation(SimulationParametersIn& simulation_parameters_in,
                             SimulationParametersOut& simulation_parameters_out);
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
    void copyStateVectorTillTimeOut(SimulationParametersIn& simulation_parameters_in,
                                      SimulationParametersOut& simulation_parameters_out);
    double cumulative_propensity_[SSA_NumReactions];
    int reaction_index_;
    int itime_;
    double time_curr_;
    double time_next_;
};

class GillespieBasic : public Gillespie
{
public:

    GillespieBasic(Logger* logger)
    :logger_(logger)
    {

    }
    ~GillespieBasic()
    {

    }
    void runSimulation(SimulationParametersIn& simulation_parameters_in,
                       SimulationParametersOut& simulation_parameters_out);
    int findNextReactionIndex(double chosen_propensity);
    double calculateExponentialRandomValue(double rand_one)
    {
        return ((1 / cumulative_propensity_[SSA_NumReactions - 1]) * log ( 1 / rand_one));
    }
    double chooseRandomPropensity(double rand_two)
    {
        return (rand_two * cumulative_propensity_[SSA_NumReactions - 1 ]);
    }
private:
    Logger* logger_;
};


class SimulationFactory
{
public:

    static Gillespie* create(Gillespie::TYPE type,Logger* logger)
    {
        switch(type)
        {
            case Gillespie::BASIC:
                return  (new GillespieBasic(logger));
            break;
            default:
                mexErrMsgIdAndTxt("SSA:InvalidGillespie",
                                  "This Gillespie simulation is not supported");
        }

    }

};



#endif //DR_SSASIM_GELLESPIE_H
