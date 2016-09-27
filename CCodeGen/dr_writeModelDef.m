function [] = dr_writeModelDef(System, ModelStringMapping)
%% Internal function - write CPP Model definition temp file
%
% Input:
% System  - Jan H. style SSA system definition struct. Ask him for details.
%           (jan.hasenauer@helmholtz-muenchen.de)
% ModelStateStrings - Output from dr_prepParser. Needed to consistently
%                     parse state names to cpp.
% ModelParamStrings - Output from dr_prepParser. Needed to consistently
%                     parse state names to cpp.
%
% Output:
% No Matlab output, output file is written to DRTB_modeldef_tmp.cpp
% Note that we *should* be in a temp folder, unless we're debugging
%
% -------------------------------------------------------------------------
% Code state: Alpha
% -Code is running
% -Did not test result correctness
% -Only tested on Linux
% -Currently no error handling
%
% -------------------------------------------------------------------------
% Notes:
% -All hail Phil, prince of insufficient light
%
% -------------------------------------------------------------------------
% Initial creation:  06.11.2014
% Last major update: 07.11.2014
% Contact: Dennis Rickert (dennis.rickert@helmholtz-muenchen.de) 
%
% Not published anywhere, under any license whatsoever. If you're using
% this without working at the ICB, you're a bad person and should feel bad

%% Core Algorithm
    fid = fopen('DRTB_modeldef_tmp.cpp','w');

    ModelCumPropStrings = dr_parseCumProps(System,ModelStringMapping);

    % Write Cpp Header
    fprintf(fid,'#include "mex.h"\n');
    fprintf(fid,'#include <cmath>\n');
    fprintf(fid,'#include <algorithm>\n');
    fprintf(fid,'#include "DRTB_modeldefHeader_tmp.hpp"\n');
    fprintf(fid,'#include "logger_tmp.hpp"\n\n');

    % Write calculateCumProps
    fprintf(fid,'int calculateCumProps(double* DRTB_CumProp, double* DRTB_State, double* DRTB_Param)\n');
    fprintf(fid,'{\n');
    for i = 1:numel(ModelCumPropStrings)
       fprintf(fid,['\t' ModelCumPropStrings{i} '\n']); 
       fprintf(fid,'\tCHECK_NOTNEG(%d, DRTB_CumProp[%d]);\n',i-1, i-1);
      

    end
    fprintf(fid,'\treturn 0;\n');
    fprintf(fid,'}\n\n');

    
    % Write updateState
    fprintf(fid,'int updateState(double* DRTB_State, int reactionID)\n');
    fprintf(fid,'{\n');
    fprintf(fid,'    switch(reactionID)\n');
    fprintf(fid,'     {\n');
    for i = 1:numel(System.reaction)
        fprintf(fid,['\t\tcase ' num2str(i) ': ']);
        
        for i2 = 1:numel(System.reaction(i).educt),
            EductStr = dr_parseSymToString(System.reaction(i).educt(i2),ModelStringMapping);
            fprintf(fid,[EductStr ' = ' EductStr ' - 1; ']);
        end
        
        for i2 = 1:numel(System.reaction(i).product),
            ProductStr = dr_parseSymToString(System.reaction(i).product(i2),ModelStringMapping);
            fprintf(fid,[ProductStr ' = ' ProductStr ' + 1; ']);
        end
        fprintf(fid,'break;\n');
        
    end
    fprintf(fid,'     }\n');
    fprintf(fid,'\treturn 0;\n');
    fprintf(fid,'}\n');
    
    fclose(fid);
end

