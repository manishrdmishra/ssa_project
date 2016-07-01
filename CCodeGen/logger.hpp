#ifndef LOG_HPP_
#define LOG_HPP_

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>

#define NUM_VARS 8
#define MAX_HISTORY 1000
#define MAX_VAR_LEN 30
enum VAR
{
	RAND_ONE = 0,
	RAND_TWO,
	T_CURR,
	T_NEXT,
	STATES,
	PROPENSITIES,
	CHOSEN_PROPENSITY,
	REACTION_INDEX
};

char NAME_OF_VAR[][MAX_VAR_LEN] =
{ "rand_one", "rand_two", "t_curr", "t_next", "state vector",
		"propensity vector", "chosen propensity", "chosen reaction index" };

std::string COLON(": ");

enum OUTPUT
{
	STD_OUTPUT = 0, FILE_OUTPUT
};
enum LOGLEVEL
{
	ALL = 0, DEBUG, INFO, OFF
};
bool shouldBeLogged(LOGLEVEL level, int *log_level_vars, VAR var)
{
	if (log_level_vars[var] >= level)
	{
		return true;
	}
	return false;
}
void initializeLoggingFlags(LOGLEVEL level, int* log_level_of_vars,
		bool* logging_flag_of_var)
{
	if (shouldBeLogged(level, log_level_of_vars, RAND_ONE) == true)
	{
		logging_flag_of_var[RAND_ONE] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, RAND_TWO) == true)
	{
		logging_flag_of_var[RAND_TWO] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, T_CURR) == true)
	{
		logging_flag_of_var[T_CURR] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, T_NEXT) == true)
	{
		logging_flag_of_var[T_NEXT] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, STATES) == true)
	{
		logging_flag_of_var[STATES] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, PROPENSITIES) == true)
	{
		logging_flag_of_var[PROPENSITIES] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, CHOSEN_PROPENSITY) == true)
	{
		logging_flag_of_var[CHOSEN_PROPENSITY] = true;
	}
	if (shouldBeLogged(level, log_level_of_vars, REACTION_INDEX) == true)
	{
		logging_flag_of_var[REACTION_INDEX] = true;
	}

}

void update_logRotation(int i, LOGLEVEL level, bool* logging_flag_of_var,
		double *log_rand_one, double *log_rand_two, double *log_t_curr,
		double* log_t_next, double log_states[][SSA_NumStates],
		double log_propensities[][SSA_NumReactions],
		double* log_choosen_propensities, double *log_reaction_indices,
		double curr_rand_one, double curr_rand_two, double curr_t_curr,
		double curr_t_next, double curr_states[], double curr_propensities[],
		double curr_choosen_propensity, double curr_reaction_index)
{

	if (logging_flag_of_var[RAND_ONE] == true)
	{
		log_rand_one[i] = curr_rand_one;
	}
	if (logging_flag_of_var[RAND_TWO] == true)
	{
		log_rand_two[i] = curr_rand_two;
	}
	if (logging_flag_of_var[T_CURR] == true)
	{
		log_t_curr[i] = curr_t_curr;
	}
	if (logging_flag_of_var[T_NEXT] == true)
	{
		log_t_next[i] = curr_t_next;
	}
	if (logging_flag_of_var[STATES] == true)
	{
		for (int j = 0; j < SSA_NumStates; j++)
		{
			log_states[i][j] = curr_states[j];
		}
	}
	if (logging_flag_of_var[PROPENSITIES] == true)
	{
		for (int j = 0; j < SSA_NumReactions; j++)
		{
			log_propensities[i][j] = curr_propensities[j];
		}
	}
	if (logging_flag_of_var[CHOSEN_PROPENSITY] == true)
	{
		log_choosen_propensities[i] = curr_choosen_propensity;
	}
	if (logging_flag_of_var[REACTION_INDEX] == true)
	{
		log_reaction_indices[i] = curr_reaction_index;
	}

}

void writeOneStep(OUTPUT method, std::ofstream& fstream, LOGLEVEL level,
		bool* logging_flag_of_var, double log_rand_one, double log_rand_two,
		double log_t_curr, double log_t_next, double log_states[],
		double log_propensities[], double log_choosen_propensities,
		double log_reaction_indices)
{

	std::stringstream stream;
	if (logging_flag_of_var[RAND_ONE] == true)
	{

		stream << NAME_OF_VAR[RAND_ONE] << COLON << log_rand_one << std::endl;

	}
	if (logging_flag_of_var[RAND_TWO] == true)
	{

		stream << NAME_OF_VAR[RAND_TWO] << COLON << log_rand_two << std::endl;

	}
	if (logging_flag_of_var[T_CURR] == true)
	{
		stream << NAME_OF_VAR[T_CURR] << COLON << log_t_curr << std::endl;

	}
	if (logging_flag_of_var[T_NEXT] == true)
	{
		stream << NAME_OF_VAR[T_NEXT] << COLON << log_t_next << std::endl;

	}
	if (logging_flag_of_var[STATES] == true)
	{
		stream << NAME_OF_VAR[STATES] << COLON;

		for (int j = 0; j < SSA_NumStates; j++)
		{
			stream << log_states[j] << "  ";

		}
		stream << std::endl;

	}
	if (logging_flag_of_var[PROPENSITIES] == true)
	{
		stream << NAME_OF_VAR[PROPENSITIES] << COLON;

		for (int j = 0; j < SSA_NumReactions; j++)
		{
			stream << log_propensities[j] << "  ";

		}
		stream << std::endl;

	}
	if (logging_flag_of_var[CHOSEN_PROPENSITY] == true)
	{
		stream << NAME_OF_VAR[CHOSEN_PROPENSITY] << COLON
				<< log_choosen_propensities << std::endl;

	}
	if (logging_flag_of_var[REACTION_INDEX] == true)
	{
		stream << NAME_OF_VAR[REACTION_INDEX] << COLON << log_reaction_indices
				<< std::endl;

	}

	if (method == STD_OUTPUT)
	{
		std::cout << stream.rdbuf();
	}
	else if (method == FILE_OUTPUT)
	{

		fstream << stream.rdbuf();
	}
}

void write_log(OUTPUT destination, std::string file_name, int i, int maxHistory,
		LOGLEVEL level, bool* logging_flag_of_var, double *log_rand_one,
		double *log_rand_two, double *log_t_curr, double* log_t_next,
		double log_states[][SSA_NumStates],
		double log_propensities[][SSA_NumReactions],
		double* log_choosen_propensities, double *log_reaction_indices)
{

	std::ofstream fstream;
	fstream.open(file_name.c_str(),
			std::ios_base::app | std::ios_base::ate | std::ios_base::out
					| std::ios_base::binary);
	for (int k = 0; k < maxHistory; k++)
	{

		writeOneStep(destination, fstream, level, logging_flag_of_var,
				log_rand_one[k], log_rand_two[k], log_t_curr[k], log_t_next[k],
				log_states[k], log_propensities[k], log_choosen_propensities[k],
				log_reaction_indices[k]);

	}
	fstream.close();
}

#endif
