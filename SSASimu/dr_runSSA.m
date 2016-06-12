function Timecourse = dr_runSSA(System,timepoints, x0, parameters, numruns) %#ok<*INUSL>
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
    dr_compileModel(System, ExecID, 1);
    for i = 1:numruns
        Timecourse(:,:,i) = dr_runSSAWithModel(timepoints, x0, parameters, ExecID); %#ok<*INUSL>
    end
    delete(which(ExecID));
    
end

