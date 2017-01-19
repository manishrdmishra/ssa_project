#include "logger_tmp.hpp"
//#include "logger.hpp"

/* assigns name to variables which will be used during logging */

char NAME_OF_VAR[][MAX_VAR_LEN] =
		{ "rand_one", "rand_two", "t_curr", "t_next", "state vector",
		  "propensity vector", "chosen propensity", "chosen reaction index" };

std::string COLON(": ");

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
 * input : log_states (An 2-d array which stores the maxHistory number of state vector
 *         i.e.number of chemical molecules in each step )
 * input : log_propensities ( An 2-d array which stores the maxHistory number of propensities
 *         vector )
 *  input : log_chosen_propensities ( An array which stores the maxHistory number of chosen propensities )
 *
 * input : log_reaction_indices ( An array which store the maxHistory number of choose reaction indices )
 *
 * input : curr_rand_one (current value of log_rand_one )
 * input : curr_rand_two ( current value of log_rand_two )
 * input : curr_t_curr ( current value of t_curr )
 * input : curr_t_next ( current value of t_next )
 * input : curr_states ( current state vector )
 * input : curr_propensities ( current propensity vector )
 * input : curr_chosen_propensity ( current value of chosen propensity )
 * input : curr_reaction_index ( current value of reaction index )
 *
 * output : void
 *
 * This function is called after each reaction to update
 * the current state of the system. The number of variables
 * updated depends on severity level of logging
 *******************************************************/
void Logger::update_logRotation(long long unsigned current_step, double curr_rand_one,
								double curr_rand_two, double curr_t_curr,
								double curr_t_next, double curr_states[],
								double curr_propensities[], double curr_chosen_propensity,
								double curr_reaction_index)
{

	switch(level_)
	{
		case OFF:
			break;
		case INFO:
			updateForLevelInfo(current_step,curr_states,
							   curr_propensities,curr_reaction_index);
			break;
		case DEBUG:
			updateForLevelDebug(current_step,curr_t_curr,curr_t_next,
								curr_states,curr_propensities,
								curr_chosen_propensity, curr_reaction_index);
			break;
		case ALL:
			updateForLevelAll(current_step,curr_rand_one,curr_rand_two, curr_t_curr,
							  curr_t_next,curr_states, curr_propensities,
							  curr_chosen_propensity,curr_reaction_index);
			break;
		default:
			mexPrintf("Log level : %d", level_);
			mexErrMsgIdAndTxt("SSA:InvalidLogginFlag",
							  "This logging flag is not supported");


	}

}


void Logger::updateForLevelInfo(long long unsigned current_step, double *curr_states,
								double *curr_propensities, double curr_reaction_index)
{
	// update states
	for (int j = 0; j < SSA_NumStates; j++)
	{
		log_states_[current_step][j] = curr_states[j];
	}
	// update propensities
	for (int j = 0; j < SSA_NumReactions; j++)
	{
		log_propensities_[current_step][j] = curr_propensities[j];
	}
	// update chosen reaction index
	log_chosen_reaction_index_[current_step] = curr_reaction_index;

}
void Logger::updateForLevelDebug(long long unsigned current_step, double curr_t_curr,
								 double curr_t_next, double *curr_states,
								 double *curr_propensities, double curr_chosen_propensity,
								 double curr_reaction_index)
{
	updateForLevelInfo(current_step,curr_states,
					   curr_propensities,curr_reaction_index);
	log_time_curr_[current_step] = curr_t_curr;
	log_time_next_[current_step] = curr_t_next;
	log_chosen_propensity_[current_step] = curr_chosen_propensity;
}
void Logger::updateForLevelAll(long long unsigned current_step, double curr_rand_one,
							   double curr_rand_two, double curr_t_curr,
							   double curr_t_next, double *curr_states,
							   double *curr_propensities, double curr_chosen_propensity,
							   double curr_reaction_index)
{
	updateForLevelDebug(current_step,curr_t_curr,curr_t_next,
						curr_states,curr_propensities,
						curr_chosen_propensity, curr_reaction_index);
	log_rand_one_[current_step] = curr_rand_one;
	log_rand_two_[current_step] = curr_rand_two;

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
 * input : log_states (current value of state vector
 *         i.e.number of chemical molecules in each step )
 * input : log_propensities ( current value of propensity
 *         vector )
 * input : log_chosen_propensities ( current value of chosen propensity )
 *
 * input : log_reaction_indices (current value of choose reaction index )
 *
 * output : void
 *
 * This function writes the current state of the system to
 * the required output.
 ****************************************************************/
void Logger::writeOneStep(OUTPUT method, std::ofstream& fstream,
						  long long unsigned current_step, double log_rand_one,
						  double log_rand_two, double log_t_curr,
						  double log_t_next, double log_states[],
						  double log_propensities[], double log_chosen_propensities,
						  double log_current_reaction_index)
{

	std::stringstream stream;

	// put the data in string stream
	stream << current_step << COLON << std::endl;
	switch(level_)
	{
		case OFF:
			break;
		case INFO:
			writeOneStepForLevelInfo(stream, log_states, log_propensities,
									 log_current_reaction_index);
			break;
		case DEBUG:
			writeOneStepForLevelDebug(stream,log_t_curr, log_t_next,log_states,
									  log_propensities, log_chosen_propensities,
									  log_current_reaction_index);
			break;
		case ALL:
			writeOneStepForLevelALL(stream, log_rand_one, log_rand_two,
									log_t_curr, log_t_next, log_states, log_propensities,
									log_chosen_propensities, log_current_reaction_index);
			break;
		default:
			mexPrintf("Log level : %d", level_);
			mexErrMsgIdAndTxt("SSA:InvalidLogginFlag",
							  "This logging flag is not supported");
	}
	// write the string stream to required output
	if (method == STD_OUTPUT)
	{
		std::cout << stream.rdbuf();
	}
	else if (method == FILE_OUTPUT)
	{
		fstream << stream.rdbuf();
	}
}


void Logger::writeOneStepForLevelInfo(std::stringstream& stream, double *log_states,
									  double *log_propensities, double log_reaction_indices)
{
	stream << NAME_OF_VAR[STATES] << COLON;

	for (int j = 0; j < SSA_NumStates; j++)
	{
		stream << log_states[j] << "  ";
	}
	stream << std::endl;

	stream<<NAME_OF_VAR[PROPENSITIES]<<COLON;

	for (int j = 0; j < SSA_NumReactions; j++)
	{
		stream << log_propensities[j] << "  ";
	}
	stream << std::endl;

	stream << NAME_OF_VAR[REACTION_INDEX] << COLON << log_reaction_indices
		   << std::endl;

}

void Logger::writeOneStepForLevelDebug(std::stringstream &stream, double log_t_curr,
									   double log_t_next, double *log_states,
									   double *log_propensities, double log_chosen_propensities,
									   double log_current_reaction_index)
{
	stream << NAME_OF_VAR[T_CURR] << COLON << log_t_curr << std::endl;
	stream << NAME_OF_VAR[T_NEXT] << COLON << log_t_next << std::endl;
	stream << NAME_OF_VAR[CHOSEN_PROPENSITY] << COLON
		   << log_chosen_propensities << std::endl;
	writeOneStepForLevelInfo(stream,log_states,
							 log_propensities,log_current_reaction_index);


}

void Logger::writeOneStepForLevelALL(std::stringstream &stream, double log_rand_one,
									 double log_rand_two, double log_t_curr,
									 double log_t_next, double *log_states,
									 double *log_propensities, double log_chosen_propensities,
									 double log_current_reaction_index)
{

	stream << NAME_OF_VAR[RAND_ONE] << COLON << log_rand_one << std::endl;
	stream << NAME_OF_VAR[RAND_TWO] << COLON << log_rand_two << std::endl;
	writeOneStepForLevelDebug(stream,log_t_curr,
							  log_t_next,log_states,log_propensities,
							  log_chosen_propensities,log_current_reaction_index);

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
 *  input : log_chosen_propensities ( An array which stores the maxHistory number of chosen propensities )
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

	// write from the current counter to the first one
	//std::cout << "current step : " << current_step << std::endl;
	for (int k = current_step; k >= 0; k--)
	{

		writeOneStep (destination, fstream, k , log_rand_one_[k],
					  log_rand_two_[k], log_time_curr_[k], log_time_next_[k],
					  log_states_[k], log_propensities_[k], log_chosen_propensity_[k],
					  log_chosen_reaction_index_[k]);

	}
	// write from the maxHistory to the (current_counter - 1)
	for (int k = logging_parameters_.num_history_ - 1; k < current_step; k--)
	{
		writeOneStep (destination, fstream, k , log_rand_one_[k],
					  log_rand_two_[k], log_time_curr_[k], log_time_next_[k],
					  log_states_[k], log_propensities_[k], log_chosen_propensity_[k],
					  log_chosen_reaction_index_[k]);
	}
}

