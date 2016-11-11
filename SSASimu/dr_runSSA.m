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
%% specify the compiler options
% To get details about these flags, please look at dr_compileModel.m file

compiler_options.cleanup = 1;
compiler_options.optimization = 1;
compiler_options.logging = 0;
compiler_options.logging_level = 0;

dr_compileModel(System, ExecID, compiler_options);
Timecourse = dr_runSSAWithModel(timepoints, x0, parameters,program_options, ExecID,numruns); %#ok<*INUSL>
delete(which(ExecID));

end

