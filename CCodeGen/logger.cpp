#include "logger_tmp.hpp"

/* assigns name to variables which will be used during logging */

char NAME_OF_VAR[][MAX_VAR_LEN] =
{ "rand_one", "rand_two", "t_curr", "t_next", "state vector",
		"propensity vector", "chosen propensity", "chosen reaction index" };

std::string COLON(": ");

/****************************************************************
 * This function returns true if the given variable should be
 *logged according to the set severity level.
 ****************************************************************/

bool shouldBeLogged(LOGLEVEL level, int *log_level_vars, VAR var)
{
	if (log_level_vars[var] >= level)
	{
		return true;
	}
	return false;
}

/****************************************************************
 *Assigns the logging flag to each variable according to
 *required log level
 ****************************************************************/
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

/*******************************************************
 * This function is called after each reaction to update
 * the current state of the system. The number of variables
 * updated depends on severity level of logging
 *******************************************************/

void update_logRotation(long long unsigned i, LOGLEVEL level,
		bool* logging_flag_of_var, double *log_rand_one, double *log_rand_two,
		double *log_t_curr, double* log_t_next,
		double log_states[][SSA_NumStates],
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

/****************************************************************
 * This function writes the current state of the system to
 * the require output.
 ****************************************************************/
void writeOneStep(OUTPUT method, std::ofstream& fstream,
		long long unsigned current_step, LOGLEVEL level,
		bool* logging_flag_of_var, double log_rand_one, double log_rand_two,
		double log_t_curr, double log_t_next, double log_states[],
		double log_propensities[], double log_choosen_propensities,
		double log_reaction_indices)
{

	std::stringstream stream;

	/* put the data in string stream */
	stream << current_step << COLON << std::endl;
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

	/* write the string stream to required output */
	if (method == STD_OUTPUT)
	{
		std::cout << stream.rdbuf();
	}
	else if (method == FILE_OUTPUT)
	{

		fstream << stream.rdbuf();
	}
}

/****************************************************************
 * This function writes the last n states of the system.
 ****************************************************************/
void writeLastNSteps(OUTPUT destination, std::ofstream& fstream,
		long long unsigned i, long long unsigned maxHistory, LOGLEVEL level,
		bool* logging_flag_of_var, double *log_rand_one, double *log_rand_two,
		double *log_t_curr, double* log_t_next,
		double log_states[][SSA_NumStates],
		double log_propensities[][SSA_NumReactions],
		double* log_choosen_propensities, double *log_reaction_indices)
{

//	std::ofstream fstream;
//	fstream.open(file_name.c_str(),
//			std::ios_base::app | std::ios_base::ate | std::ios_base::out
//					| std::ios_base::binary);
	/* write from the current counter to the first one */
	int temp = maxHistory;
	std::cout << "current step : " << i << std::endl;
	for (int k = i; k >= 0; k--)
	{/*TODO: Bug - Change this as this will crash for DEBUG/INFO level as we are trying
	 to access indices on null pointers ex: log_rand_one/log_rand_two

	 can be null for DEBUG case */
#ifdef LEVEL_ALL
		writeOneStep (destination, fstream, temp--, level, logging_flag_of_var,
				log_rand_one[k], log_rand_two[k], log_t_curr[k], log_t_next[k],
				log_states[k], log_propensities[k], log_choosen_propensities[k],
				log_reaction_indices[k]);
#elif LEVEL_DEBUG
		writeOneStep (destination, fstream, temp--, level, logging_flag_of_var,
				NULL, NULL, log_t_curr[k], log_t_next[k],
				log_states[k], log_propensities[k],NULL,
				log_reaction_indices[k]);
#elif LEVEL_INFO
		writeOneStep (destination, fstream, temp--, level, logging_flag_of_var,
				NULL, NULL, NULL, NULL,
				log_states[k], log_propensities[k],NULL,
				log_reaction_indices[k]);
#endif

	}
	/* write from the maxHistory to the (current_counter - 1) */
	for (int k = maxHistory; k < i; k--)
	{
#ifdef LEVEL_ALL
		writeOneStep (destination, fstream, temp--, level, logging_flag_of_var,
				log_rand_one[k], log_rand_two[k], log_t_curr[k], log_t_next[k],
				log_states[k], log_propensities[k], log_choosen_propensities[k],
				log_reaction_indices[k]);
#elif LEVEL_DEBUG
		writeOneStep (destination, fstream, temp--, level, logging_flag_of_var,
				NULL, NULL, log_t_curr[k], log_t_next[k],
				log_states[k], log_propensities[k],NULL,
				log_reaction_indices[k]);
#elif LEVEL_INFO
		writeOneStep (destination, fstream, temp--, level, logging_flag_of_var,
				NULL, NULL, NULL, NULL,
				log_states[k], log_propensities[k],NULL,
				log_reaction_indices[k]);
#endif

	}
	fstream.close();
}
/****************************************************************
 * This function open the output stream for writing to corresponding
 * given file.
 ****************************************************************/
void openOutputStream(std::string file_name, std::ofstream& fstream)
{

	if (fstream.is_open() == false)
	{
		fstream.open(file_name.c_str(),
				std::ios_base::app | std::ios_base::ate | std::ios_base::out
						| std::ios_base::binary);
	}
}

/****************************************************************
 * This function close the output stream.
 ****************************************************************/
void closeOutputStream(std::ofstream& fstream)
{
	if (fstream.is_open() == false)
	{
		fstream.close();
	}

}
