#include "logger_tmp.hpp"

/* assigns name to variables which will be used during logging */

char NAME_OF_VAR[][MAX_VAR_LEN] =
{ "rand_one", "rand_two", "t_curr", "t_next", "state vector",
		"propensity vector", "chosen propensity", "chosen reaction index" };

std::string COLON(": ");

/****************************************************************
 * input : level ( logging level )
 * input : log_level_of_vars (An array which contains the logging 
 *         level of state variable which is fixed ) 
 *
 * output : returns true if this variable should be logged else false
 *
 * This function returns true if the given variable should be
 *logged according to the set severity level.
 ****************************************************************/

bool Logger::shouldBeLogged(VAR var)
{
	if (log_level_of_var_[var] >= level_)
	{
		return true;
	}
	return false;
}

/****************************************************************
 *
 * input : level ( logging level )
 * input : log_level_of_vars (An array which contains the logging 
 *         level of state variable which is fixed ) 
 * input : logging_flag_of_var ( provides information about the logging level
 *         for each state variable )
 * output : void 
 *
 *Assigns the logging flag to each variable according to
 *required log level. 
 ****************************************************************/
void Logger::initializeLoggingFlags()
{
	if (shouldBeLogged( RAND_ONE) == true)
	{
		logging_flag_of_var_[RAND_ONE] = true;
	}
	if (shouldBeLogged( RAND_TWO) == true)
	{
		logging_flag_of_var_[RAND_TWO] = true;
	}
	if (shouldBeLogged( T_CURR) == true)
	{
		logging_flag_of_var_[T_CURR] = true;
	}
	if (shouldBeLogged(T_NEXT) == true)
	{
		logging_flag_of_var_[T_NEXT] = true;
	}
	if (shouldBeLogged( STATES) == true)
	{
		logging_flag_of_var_[STATES] = true;
	}
	if (shouldBeLogged( PROPENSITIES) == true)
	{
		logging_flag_of_var_[PROPENSITIES] = true;
	}
	if (shouldBeLogged( CHOSEN_PROPENSITY) == true)
	{
		logging_flag_of_var_[CHOSEN_PROPENSITY] = true;
	}
	if (shouldBeLogged( REACTION_INDEX) == true)
	{
		logging_flag_of_var_[REACTION_INDEX] = true;
	}

}

/*******************************************************
 *
 * input : current_step ( current counter )
 * input : level ( logging level )
 * input : logging_flag_of_var ( provides information about the logging level
 *         for each state variable )
 * input : log_rand_one ( An array which stores the maxHistory number of log_rand_one values)
 * input : log_rand_two ( An array which stroes the maxHistory number of log_rand_two values)
 * input : log_t_curr ( An array which stores the maxHistory number of t_curr values )
 * input : log_t_next ( An array which stores the maxHistory number of t_next values )
 * input : log_states (An 2-d array which stores the maxHistory number of state vecotor 
 *         i.e.number of chemical molecules in each step )
 * input : log_propensities ( An 2-d array which stores the maxHistory number of propesnsities
 *         vector )
 *  input : log_choosen_propensitie ( An array which stores the maxHistory number of choosen propesnsities )
 *
 * input : log_reaction_indices ( An array which store the maxHistory number of choose reaction indices )
 *
 * input : curr_rand_one (current value of log_rand_one )
 * input : curr_rand_two ( current value of log_rand_two )
 * input : curr_t_curr ( current value of t_curr )
 * input : curr_t_next ( current value of t_next )
 * input : curr_states ( current state vector )
 * input : curr_propensities ( current propensity vector )
 * input : curr_choosen_propensity ( current value of choosen propensity )
 * input : curr_reaction_index ( current value of reaction index )
 *
 * output : void
 *
 * This function is called after each reaction to update
 * the current state of the system. The number of variables
 * updated depends on severity level of logging
 *******************************************************/

void Logger::update_logRotation(long long unsigned current_step, double curr_rand_one, double curr_rand_two, double curr_t_curr,
		double curr_t_next, double curr_states[], double curr_propensities[],
		double curr_choosen_propensity, double curr_reaction_index)
{

//	history_counts_ = history_counts_ + 1;
//	if (history_counts_ > logging_parameters_.num_history_)
//	{
//		history_counts_ = 0;
//	}

	if (logging_flag_of_var_[RAND_ONE] == true)
	{
		log_rand_one_[current_step] = curr_rand_one;
		//std::cout<<"updating log_rand_one...\n"<<std::endl;
	}
	if (logging_flag_of_var_[RAND_TWO] == true)
	{
		log_rand_two_[current_step] = curr_rand_two;
	}
	if (logging_flag_of_var_[T_CURR] == true)
	{
		log_time_curr_[current_step] = curr_t_curr;
	}
	if (logging_flag_of_var_[T_NEXT] == true)
	{
		log_time_next_[current_step] = curr_t_next;
		//std::cout<<"updating t_curr...\n"<<std::endl;
	}
	if (logging_flag_of_var_[STATES] == true)
	{
		for (int j = 0; j < SSA_NumStates; j++)
		{
			log_states_[current_step][j] = curr_states[j];
		}
	}
	if (logging_flag_of_var_[PROPENSITIES] == true)
	{
		for (int j = 0; j < SSA_NumReactions; j++)
		{
			log_propensities_[current_step][j] = curr_propensities[j];
		}
	}
	if (logging_flag_of_var_[CHOSEN_PROPENSITY] == true)
	{
		log_chosen_propensity_[current_step] = curr_choosen_propensity;
	}
	if (logging_flag_of_var_[REACTION_INDEX] == true)
	{
		log_chosen_reaction_index_[current_step] = curr_reaction_index;
	}

}

/****************************************************************
 * input : destination ( where output will be written )
 * input : fstream ( A file stream )
 * input : current_step ( current counter )
 * input : maxHistory ( max number of steps to be saved for history )
 * input : level ( logging level )
 * input : logging_flag_of_var ( provides information about the logging level
 *         for each state variable )
 * input : log_rand_one ( current value of log_rand_one )
 * input : log_rand_two ( current value of log_rand_two )
 * input : log_t_curr ( current value of t_curr  )
 * input : log_t_next ( current value of t_next  )
 * input : log_states (current value of state vecotor 
 *         i.e.number of chemical molecules in each step )
 * input : log_propensities ( current value of propesnsity
 *         vector )
 * input : log_choosen_propensitie ( current value of choosen propesnsity )
 *
 * input : log_reaction_indices (current value of choose reaction index )
 *
 * output : void
 *
 * This function writes the current state of the system to
 * the required output.
 ****************************************************************/
void Logger::writeOneStep(OUTPUT method, std::ofstream& fstream,
		long long unsigned current_step, double log_rand_one, double log_rand_two,
		double log_t_curr, double log_t_next, double log_states[],
		double log_propensities[], double log_choosen_propensities,
		double log_reaction_indices)
{

	std::stringstream stream;

	/* put the data in string stream */
	stream << current_step << COLON << std::endl;
	if (logging_flag_of_var_[RAND_ONE] == true)
	{

		stream << NAME_OF_VAR[RAND_ONE] << COLON << log_rand_one << std::endl;

	}
	if (logging_flag_of_var_[RAND_TWO] == true)
	{

		stream << NAME_OF_VAR[RAND_TWO] << COLON << log_rand_two << std::endl;

	}
	if (logging_flag_of_var_[T_CURR] == true)
	{
		stream << NAME_OF_VAR[T_CURR] << COLON << log_t_curr << std::endl;

	}
	if (logging_flag_of_var_[T_NEXT] == true)
	{
		stream << NAME_OF_VAR[T_NEXT] << COLON << log_t_next << std::endl;

	}
	if (logging_flag_of_var_[STATES] == true)
	{
		stream << NAME_OF_VAR[STATES] << COLON;

		for (int j = 0; j < SSA_NumStates; j++)
		{
			stream << log_states[j] << "  ";

		}
		stream << std::endl;

	}
	if (logging_flag_of_var_[PROPENSITIES] == true)
	{
		stream << NAME_OF_VAR[PROPENSITIES] << COLON;

		for (int j = 0; j < SSA_NumReactions; j++)
		{
			stream << log_propensities[j] << "  ";

		}
		stream << std::endl;

	}
	if (logging_flag_of_var_[CHOSEN_PROPENSITY] == true)
	{
		stream << NAME_OF_VAR[CHOSEN_PROPENSITY] << COLON
				<< log_choosen_propensities << std::endl;

	}
	if (logging_flag_of_var_[REACTION_INDEX] == true)
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
 *
 * input : destination ( where output will be written )
 * input : fstream ( A file stream )
 * input : current_step ( current counter )
 * input : maxHistory ( max number of steps to be saved for history )
 * input : level ( logging level )
 * input : logging_flag_of_var ( provides information about the logging level
 *         for each state variable )
 * input : log_rand_one ( An array which stores the maxHistory number of log_rand_one values)
 * input : log_rand_two ( An array which stroes the maxHistory number of log_rand_two values)
 * input : log_t_curr ( An array which stores the maxHistory number of t_curr values )
 * input : log_t_next ( An array which stores the maxHistory number of t_next values )
 * input : log_states (An 2-d array which stores the maxHistory number of state vecotor 
 *         i.e.number of chemical molecules in each step )
 * input : log_propensities ( An 2-d array which stores the maxHistory number of propesnsities
 *         vector )
 *  input : log_choosen_propensitie ( An array which stores the maxHistory number of choosen propesnsities )
 *
 * input : log_reaction_indices ( An array which store the maxHistory number of choose reaction indices )
 *
 * output : void
 *
 * This function writes the last n states of the system to the given output.
 ****************************************************************/
void Logger::writeLastNSteps(OUTPUT destination, std::ofstream& fstream,
		long long unsigned current_step)
{


	/* write from the current counter to the first one */
	//int temp = maxHistory;
	//std::cout << "current step : " << current_step << std::endl;
	for (int k = current_step; k >= 0; k--)
	{

//#ifdef LEVEL_ALL
		writeOneStep (destination, fstream, k ,
				log_rand_one_[k], log_rand_two_[k], log_time_curr_[k], log_time_next_[k],
				log_states_[k], log_propensities_[k], log_chosen_propensity_[k],
				log_chosen_reaction_index_[k]);
/* #elif LEVEL_DEBUG
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
*/
	}
	/* write from the maxHistory to the (current_counter - 1) */
	for (int k = logging_parameters_.num_history_ - 1; k < current_step; k--)
	{
//#ifdef LEVEL_ALL
		writeOneStep (destination, fstream, k ,
				log_rand_one_[k], log_rand_two_[k], log_time_curr_[k], log_time_next_[k],
				log_states_[k], log_propensities_[k], log_chosen_propensity_[k],
				log_chosen_reaction_index_[k]);
/*#elif LEVEL_DEBUG
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
*/
	}
	fstream.close();
}
/****************************************************************
 *
 * input : file_name ( An String )
 * input : fstream  ( A file stream )
 *
 * output : void 
 *
 * This function open the output stream for writing to corresponding
 * given file.
 ****************************************************************/
void Logger::openFileStream(std::string file_name, std::ofstream& fstream)
{

	if (fstream.is_open() == false)
	{
		fstream.open(file_name.c_str(),
				std::ios_base::app | std::ios_base::ate | std::ios_base::out
						| std::ios_base::binary);
	}
}

/****************************************************************
 *
 * input : fstream ( A file stream )
 * output : void 
 *
 * This function close the output stream.
 ****************************************************************/
void Logger::closeFileStream(std::ofstream& fstream)
{
	if (fstream.is_open() == false)
	{
		fstream.close();
	}

}


/**************************************************************************
 *
 *  This function allocates memory for storing debugging information
 *  Memory is allocated on the basis of debugging level.
 *
 **************************************************************************/

/*void ssaCalloc(double **debugStateVar, mwSize n)
{
	**debugStateVar = (double *) mxCalloc(n, sizeof(double));
}


void ssaCalloc(double ***debugStateVar, mwSize n , mwSize m )
{

   


}
 */
