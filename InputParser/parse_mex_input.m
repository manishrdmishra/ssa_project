function [ parsed_options ] = parse_mex_input( mex_input_options )
%% PARSE_MEX_INPUT Summary of this function goes here
% Input:  mex_input_options - It is a structure which consists of the 
%                             following variables.
%
%  panic_file_name - This string variable contains the name of file,
%  				  which stores the state of the simulation when an error
%  				  occurs and simulation can not proceed further.
%
%  periodic_file_name  - This string variable contains the name of file,
%  					  which stores the simulation states periodically.
%
%  max_history - When simulation runs then we store the last n states of
%  			  the simulation. When some error occurs then we print
%  			  these last n states in to panic_file_name. So n is assigned
%  			  the value provided by max_history.
%
%  period      -  After x steps the state of the simulation is written to
%  			   a file periodic_file_name. so x is assigned the value
%  			   provided by period.
% Output: parsed mex input
%   Detailed explanation goes here
%% 

%   panic_file_name  =  {A valid file name },  default - 'panic_log.txt';
%   periodic_file_name = { A valid file name }, default - 'periodic_log.txt';
%   max_history = {An integer }, default - 100;
%   period = {An integer} , default - 1000;


% Get the instance of input parse
p = inputParser;

% set the default value for cleanup 
default = 'panic_log.txt';
addOptional(p,'panic_file_name',default,@ischar);

%set the default value for optimization 
default = 'periodic_log.txt';
addOptional(p,'periodic_file_name',default,@ischar);
%set the default value for logging
default = cast(100,'uint64');
addOptional(p,'max_history',default,@isinteger);

%set the default value for logging_level
default = cast(1000,'uint64');
addOptional(p,'period',default,@isinteger);

%set the default value for num_of_threads
default = cast(1,'uint64');
addOptional(p,'num_of_threads',default,@isinteger);

% parse the input argument compilter_options
% It also Fills the missing parameters by default values 
parse(p,mex_input_options)

% Assign the the results to options in order as in mex file 
% we expect them in the order, otherwise there will be class 
% mis-match error will occur. 
parsed_options.panic_file_name = p.Results.panic_file_name;
parsed_options.periodic_file_name = p.Results.periodic_file_name;
parsed_options.max_history = p.Results.max_history;
parsed_options.period = p.Results.period;
parsed_options.num_of_threads = p.Results.num_of_threads;

end

