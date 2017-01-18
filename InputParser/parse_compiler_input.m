function [ parsed_options ] = parse_compiler_input(compiler_options )

%% Parse and validate the compiler options 
% Input:
% compiler_options - it consists of following fields
%   1) cleanup -                By default, temp files (c++ source) and folder
%                               will be deleted after compilation.
%                               By specifying cleanup = 0, you can tell the
%                               function to keep temp files, e.g. for debugging causes.
%                               This function is a very well behaved function and will
%                               almost certainly obey.
%   2) compiler_optimization - if set to 1 then aggressively optimiztion is
%                              done else not. In case of aggressive
%                              optimization there might be some cases when
%                              the porgram execution is not predictable.
%    3) logging             -  If this flag is set to 1 then logging will
%                               be enabled, if set to 0 then logging will
%                               be disabled. There are two types of logging
%                               are provided
%                               1  - when there is some error during the s
%                               imulation then the last n steps of
%                               simulation will be logged to a file.
%                               2 - Periodically the steps of simulation will
%                               be logged to a file.
%                               in the simulation then
%    4) logging_level    -      This is used to set the verbosity of
%                               logging information. Lower the value , higher
%                               the verbosity of logging. If set to 0 then
%                               all the state variables are logged, if set
%                               to 3 then there will be no logging at all.
%                               0 - Verbosity is set to ALL.
%                               1 - Verbosity is set to DEBUG.
%                               2 - Verbosity is set to INFO.
%                               3 - Verbosity is set to OFF.
%                               Definition of log levels
%                               INFO - { STATES,PROPENSITIES, REACTION_INDICIES} */
%                               /* DEBUG - {T_CURR ,T_NEXT , CHOOSEN_PROPENSITY }  + INFO */
%                               /* ALL - {RAND_ONE , RAND_TWO } + DEBUG */
% Output: parsed compiler options


% parse input arguments

% cleanup =  {0,1} , default - 1
% optimization = {0,1}, default - 0
% logging = {0,1} , default - 0
% logging_level = [0 3] , default - 3
% num_of_threads = [1, max_num_of_threads], default - 1


% Get the instance of input parse
p = inputParser;

% set the default value for cleanup
default_cleanup_value = cast(1,'uint8');
addParameter(p,'cleanup',default_cleanup_value,@isinteger);

%set the default value for optimization
default_optimization_value = cast(0,'uint8');
addParameter(p,'optimization',default_optimization_value,@isinteger);

%set the default value for logging
default_logging_value = cast(0,'uint8');
addParameter(p,'logging',default_logging_value,@isinteger);

%set the default value for logging_level
default_logging_level_value = cast(4,'uint8');
addParameter(p,'logging_level',default_logging_level_value,@isinteger);

% set the default value for num_of_threads
default_num_threads = cast(1,'uint8');
addParameter(p,'num_of_threads',default_num_threads,@isinteger);

% parse the input argument compilter_options
parse(p,compiler_options)

% Assign the parsed input to options.
% This way if user does not set some flags
% then default value will be set.

    parsed_options = p.Results(:);


end

