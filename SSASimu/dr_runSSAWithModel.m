function Timecourse = dr_runSSAWithModel(timepoints, x0, parameters, program_options, ExecID,numruns) %#ok<*INUSL>
%% Run SSA simulation for compiled model without re-compiling
%
% Input:
% timepoints - Vector of desired output timepoints, should be ordered
% x0         - Vector of initial conditions, needs to use the same ordering
%              as System.state.variable of creating struct
% parameters - Vector of model parameters, needs to use the same ordering
%              as System.parameter.variable of creating struct
% ExecID     - Name of SSA executable (e.g. produced with dr_compileModel)
%              Executable needs to be on Matlab Path
% numruns    - Number of replicates desired
% program_options - It is a structure which is used to set the variables
%                   used to configure mex executable. 
% Timecourse - NxMxZ array, where N = #States, M = #timepoints and
% Z = numruns
%
% -------------------------------------------------------------------------
% Initial creation:  06.11.2014
% Last major update: 07.11.2014
% Contact: Dennis Rickert (dennis.rickert@helmholtz-muenchen.de)
%
% Not published anywhere, under any license whatsoever. If you're using
% this without working at the ICB, you're a bad person and should feel bad

%% parse program_options 

options = parse_mex_input(program_options);

%% Core Algorithm
% Initialization
% +0 is used to force matlab to create deep copies of variables, i.e. copy
% values. By default, matlab only copies the memory location and creates
% deep copies when the variable is changed; however, Matlab doesn't notice
% manipulation that happens in compiled mex files, resulting in hilarious
% bugs avoided by adding +0.
Timecourse      = zeros(length(timepoints),length(x0),numruns);
x               = x0+0; %#ok<*NASGU> %

% Calculation of entire trajectory
for i = 1:numruns
    temp = eval([ExecID '(x,parameters,timepoints,numel(timepoints),options)']);%#ok<*ASGLU>
    Timecourse(:,:,i) = reshape(temp, numel(timepoints),numel(x));
end

end

