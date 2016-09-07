function Timecourse = dr_runSSA(System,timepoints, x0, parameters, program_options, numruns) %#ok<*INUSL>
%% Run compilation and SSA simulation for model
%
% Input:
% System     - Jan H. style SSA system definition struct. Ask him for
%              details (jan.hasenauer@helmholtz-muenchen.de)
% timepoints - Vector of desired output timepoints, should be ordered
% x0         - Vector of initial conditions, needs to use the same ordering
%              as System.state.variable of creating struct
% parameters - Vector of model parameters, needs to use the same ordering
%              as System.parameter.variable of creating struct
% numruns    - Number of replicates desired
% program_options - It is a structure which is used to set the following variables.
% 		 panic_file_name - string
% 		 periodic_file_name - string
% 		 max_history  - uint64
% 		 period - unit64
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
%
% Output:
% Timecourse - NxMxZ array, where N = #States, M = #timepoints and
% Z = numruns
%
% -------------------------------------------------------------------------
% Initial creation:  07.11.2014
% Last major update: 08.11.2014
% Contact: Dennis Rickert (dennis.rickert@helmholtz-muenchen.de)
%
% Not published anywhere, under any license whatsoever. If you're using
% this without working at the ICB, you're a bad person and should feel bad

%% parse program_options 

%   panic_file_name  =  {A valid file name },  default - 'panic_log.txt';
%   periodic_file_name = { A valid file name }, default - 'periodic_log.txt';
%   max_history = {An integer }, default - 100;
%   period = {An integer} , default - 1000;


% Get the instance of input parse
p = inputParser;

% set the default value for cleanup 
default = 'panic_log.txt';
addOptional(p,'panic_file_name',default);

%set the default value for optimization 
default = 'periodic_log.txt';
addOptional(p,'periodic_file_name',default)

%set the default value for logging
default = cast(100,'uint64');
addOptional(p,'max_history',default);

%set the default value for logging_level
default = cast(1000,'uint64');
addOptional(p,'period',default);

% parse the input argument compilter_options
% It also Fills the missing parameters by default values 
parse(p,program_options)

% Assign the the results to options in order as in mex file 
% we expect them in the order, otherwise there will be class 
% mis-match error will occur. 
options.panic_file_name = p.Results.panic_file_name;
options.periodic_file_name = p.Results.periodic_file_name;
options.max_history = p.Results.max_history;
options.period = p.Results.period;


%% Core Algorithm
if ~exist('numruns','var')
    numruns = 1;
end

% Generate random execution id, avoid clash with existing files
chars    = ['a':'z' 'A':'Z' '0':'9'];
strLenth = 25;
ExecID = '';
while exist([ExecID '_tmp'],'file') || exist(ExecID,'file') || numel(ExecID) == 0
    strVec   = randi(numel(chars),[1 strLenth]);
    ExecID   = chars(strVec);
end

% Compile model, execute simulation, burn carthago
dr_compileModel(System, ExecID, 1);
Timecourse = dr_runSSAWithModel(timepoints, x0, parameters,options, ExecID,numruns); %#ok<*INUSL>
delete(which(ExecID));

end

