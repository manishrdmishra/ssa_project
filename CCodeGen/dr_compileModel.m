function [] = dr_compileModel(System, ExecID, compiler_options)
%% Compile SSA-executable from Jan H.-style SSA System definition
%
% Input:
% System  - Jan H. style SSA system definition struct. Ask him for details.
%           (jan.hasenauer@helmholtz-muenchen.de)
% ExecID  - Name desired for the executable. Note that a temp folder
%           ExecID_tmp will be created. Should this folder already exist,
%           files might be overwritten. Confusion could commence.
% compiler_options -
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
%
% Output:
% No Matlab output. Compiled executable is placed at current location.
% Executable will be named ExecID, appended by the system appropriate
% ending for mex-executables.
%
% -------------------------------------------------------------------------
% Initial creation:  06.11.2014
% Last major update: 07.11.2014
% Contact: Dennis Rickert (dennis.rickert@helmholtz-muenchen.de)
%
% Not published anywhere, under any license whatsoever. If you're using
% this without working at the ICB, you're a bad person and should feel bad

%% Core Algorithm
% parse input arguments

% cleanup =  {0,1} , default - 1
% optimization = {0,1}, default - 0
% logging = {0,1} , default - 0
% logging_level = [0 3] , default - 3
% num_of_threads = [1, max_num_of_threads], default - 1


% Get the instance of input parse
p = inputParser;

% set the default value for cleanup
default = 1;
addParameter(p,'cleanup',default);

%set the default value for optimization
default = 0;
addParameter(p,'optimization',default)

%set the default value for logging
default = 0;
addParameter(p,'logging',default);

%set the default value for logging_level
default = 4;
addParameter(p,'logging_level',default);

% set the default value for num_of_threads
default = 1;
addParameter(p,'num_of_threads',default);

% parse the input argument compilter_options
parse(p,compiler_options)

% Assign the parsed input to options.
% This way if user does not set some flags
% then default value will be set.

    options = p.Results(:);



% Create temporary working directory
curDir = pwd;
if ~exist([pwd() filesep() ExecID '_tmp'],'dir')
mkdir([ExecID '_tmp']);
end
cd([ExecID '_tmp']);

% Prepare sym -> string parser
ModelStringMapping = dr_prepParser( System);

% Write CPP files
dr_writeModelDef(System,ModelStringMapping,options.num_of_threads); % 'DRTB_modeldef_tmp.cpp'
dr_writeModelDefHeader(System);              % 'DRTB_modeldefHeader_tmp.hpp'
copyfile(which('DRTB_simulateSSA.cpp'),'DRTB_simulateSSA_tmp.cpp');
copyfile(which('logger.hpp'),'logger_tmp.hpp');
copyfile(which('logger.cpp'),'logger_tmp.cpp');
copyfile(which('program_option_parser.hpp'),'program_option_parser_tmp.hpp');
copyfile(which('gillespie.hpp'),'gillespie_tmp.hpp');
copyfile(which('gillespie.cpp'),'gillespie_tmp.cpp');


%% Compiler flags
flags.cc = {};
flags.cxx = {} ;
flags.cxxflags = {};
flags.cxxoptim = {};
flags.ldoptim = {};
flags.ldflags = {};
flags.linkpass = {};
flags.ldflags = {};


flags.cc{end+1} = '-v' ;
flags.cc{end + 1} = '-cxx';
flags.cxx{end+1} = 'g++' ;


%flags.cxxflags{end + 1} = '-std=c++11';
flags.cxxflags{end + 1} = '--ansi';
flags.cxxflags{end + 1} = '-D_GNU_SOURCE';
flags.cxxflags{end + 1} = '-fPIC';
flags.cxxflags{end + 1} = '-fno-omit-frame-pointer';
%flags.cxxflags{end + 1 } = '-pg'
flags.cxxflags{end + 1} = '-fopenmp';

flags.ldflags{end + 1} = '-pthread';
flags.ldflags{end + 1} = '-shared';
flags.ldflags{end + 1} = '-g';
flags.ldflags{end + 1} = '-Wl,--no-undefined';
%flags.ldflags{end + 1 } = '-pg';
flags.ldflags{end + 1 } = '-fopenmp';


% set the debug level flags specified by user
if(options.logging_level == 0)
flags.cxxflags{end + 1} = '-DLEVEL_ALL';
elseif(options.logging_level == 1)
flags.cxxflags{end + 1} = '-DLEVEL_DEBUG';
elseif(options.logging_level == 2)
flags.cxxflags{end + 1} = '-DLEVEL_INFO';

end

% set the flag to enable the logging
if(options.logging == 1)
flags.cxxflags{end + 1} = '-DLOGGING';
end

% set the optimization flags
if(options.optimization == 1)
disp('optimization flag is set');
flags.cxxoptim{end + 1} = '-O3';
flags.cxxoptim{end + 1} = '-ffast-math';
else
disp('optimization flag is not set');
flags.cxxoptim{end + 1} = '-O0';
flags.cxxoptim{end + 1} = '-funroll-all-loops';
end
flags.cxxoptim{end + 1}  = '-fopenmp';


flags.ldoptim{end + 1} = '-fopenmp';

%% set source files
src = {};
src{end + 1} =  fullfile(pwd,'DRTB_simulateSSA_tmp.cpp');
src{end + 1} = fullfile(pwd,'DRTB_modeldef_tmp.cpp');
src{end + 1} = fullfile(pwd,'logger_tmp.cpp');
src{end + 1} = fullfile(pwd,'gillespie_tmp.cpp');

%% concatenate the flags
flags.mexcc = horzcat(flags.cc, ...
{['CXXOPTIMFLAGS=' strjoin(flags.cxxoptim)]},...
{['CXX=' strjoin(flags.cxx)]},...
{['CXXFLAGS=' strjoin(flags.cxxflags)]}, ...
{['LDOPTIMFLAGS=' strjoin(flags.ldoptim)]},...
{['LDFLAGS=' strjoin(flags.ldflags)]});


mopts ={flags.mexcc{:},src{:}};


%% compile and link the source files
mex(mopts{:});


% Change back to start directory; move compiled file;
% clean up tempfiles (if asked to)

[~,~,MexExt] = fileparts(which('DRTB_simulateSSA_tmp'));
cd(curDir);
movefile([ExecID '_tmp/DRTB_simulateSSA_tmp' MexExt],[ExecID  MexExt]);
if (compiler_options.cleanup == 1)
    rmdir([ExecID '_tmp'],'s');
end
end
