function [] = dr_writeModelDefHeader(System)
%% Internal function - write CPP Model definition header temp file
%
% Input:
% System  - Jan H. style SSA system definition struct. Ask him for details.
%           (jan.hasenauer@helmholtz-muenchen.de)
%
% Output:
% No Matlab output, output file is written to DRTB_modeldefHeader_tmp.hpp
% Note that we *should* be in a temp folder, unless we're debugging
%
% -------------------------------------------------------------------------
% Code state: Alpha
% -Code is running
% -Did not test result correctness
% -Only tested on Linux
% -Currently no sanity checks on input / error handling
%
% -------------------------------------------------------------------------
% Notes:
%
% -------------------------------------------------------------------------
% Initial creation:  06.11.2014
% Last major update: 07.11.2014
% Contact: Dennis Rickert (dennis.rickert@helmholtz-muenchen.de) 
%
% Not published anywhere, under any license whatsoever. If you're using
% this without working at the ICB, you're a bad person and should feel bad

%% Core Algorithm
    % We're really just writing the header file, nothing fancy.
    % Number of states etc. is parsed from system.
    % Nothing else to see, move along.
    % Now please.
    % Shoooo!
    fid = fopen('DRTB_modeldefHeader_tmp.hpp','w');
    fprintf(fid,'#ifndef __DRTB_SSAModelDef_H_INCLUDED__\n');
    fprintf(fid,'#define __DRTB_SSAModelDef_H_INCLUDED__\n\n');
    fprintf(fid,['#define SSA_NumStates     ' num2str(numel(System.state.variable))     '\n']);
    fprintf(fid,['#define SSA_NumReactions  ' num2str(numel(System.reaction))           '\n']);
    fprintf(fid,['#define SSA_NumParameters ' num2str(numel(System.parameter.variable)) '\n']);
    fprintf(fid,'void calculateCumProps(double* DRTB_CumProp, double* DRTB_State, double* DRTB_Param);\n');
    fprintf(fid,'void updateState(double* DRTB_State, int reactionID);\n\n');
    fprintf(fid,'#endif\n');
    fclose(fid);
end

