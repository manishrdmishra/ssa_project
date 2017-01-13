function [] = dr_writeModelDef(System, ModelStringMapping,num_of_threads)
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

% Get the instance of input parse
%p = inputParser;

% set the default value for cleanup
%default = 1;
%addParameter(p,'num_of_threads',default);

% parse the input argument compilter_options
%parse(p,options)

% Assign the parsed input to options.
% This way if user does not set some flags
% then default value will be set.

%   options = p.Results(:);
%% Core Algorithm
fid = fopen('DRTB_modeldef_tmp.cpp','w');

ModelCumPropStrings = dr_parseCumProps(System,ModelStringMapping);

% Write Cpp Header
fprintf(fid,'#include "mex.h"\n');
fprintf(fid,'#include <cmath>\n');
fprintf(fid,'#include <algorithm>\n');
fprintf(fid,'#include <omp.h>\n');
fprintf(fid,'#include "DRTB_modeldefHeader_tmp.hpp"\n');
fprintf(fid,'#include "logger_tmp.hpp"\n\n');

% Write calculateCumProps
fprintf(fid,'int calculateCumProps(double* DRTB_CumProp, double* DRTB_State, double* DRTB_Param)\n');
fprintf(fid,'{\n');

fprintf(fid,'\t int ret_val = 0;\n');
% start the parallel sections
fprintf(fid,'\t #pragma omp parallel sections\n');
fprintf(fid,'\t{\n');

% start the first parallel section
fprintf(fid,'\t\t#pragma omp section\n');
fprintf(fid,'\t\t{\n');


numberOfParallelSections = floor( numel(ModelCumPropStrings) / num_of_threads );
currentParallelSection = 0;

% This loop writes propensity expressions corresponding to each
% reaction. It also group them in parallel sections. if there are
% num_reactions reactions and num_processors processors then number
% of parallel sections would be num_parallel_sections ( num_reactions / num_processors ) .
% First ( num_parallel_sections - 1 ) sections would contain
% num_parallel_sections many reactions and last one would contain
% num_parallel_sections + (  num_reactions % num_processors ) .
% For example If there are 2  processors and 5 reactions then
% number of parallel section would be 2 ( 5/2 ). So reactions R1
% and R2 are placed in first parallel section and R3, R4 , R5 would
% be placed in second parallel section.

for i = 1:numel(ModelCumPropStrings)
    
    if  (canBeWrittenToCurrentParallelSection(currentParallelSection,i,numberOfParallelSections) == 1)
        writePropensityString(fid,ModelCumPropStrings,i);
    elseif (createNewParralelSection(currentParallelSection,numel(ModelCumPropStrings),numberOfParallelSections) == 1)
        currentParallelSection = currentParallelSection + 1;
        fprintf(fid,'\t\t}\n');
        fprintf(fid,'\t\t#pragma omp section\n');
        fprintf(fid,'\t\t{\n');
    else
        writePropensityString(fid,ModelCumPropStrings,i);
    end
    
end


% close the last parallel section
fprintf(fid, '\t\t}\n');
% close the parallel sections
fprintf(fid,'\t}\n');

fprintf(fid,'\treturn ret_val;\n');
fprintf(fid,'}\n\n');


% Write updateState
fprintf(fid,'int updateState(double* DRTB_State, int reactionID)\n');
fprintf(fid,'{\n');
fprintf(fid,'    switch(reactionID)\n');
fprintf(fid,'     {\n');
for i = 1:numel(System.reaction)
   
    fprintf(fid,['\t\tcase ' num2str(i) ': ']);
    writeEductString(fid,System.reaction(i).educt, ModelStringMapping)
    writeProductString(fid,System.reaction(i).product, ModelStringMapping)
    fprintf(fid,'break;\n');
    
end
fprintf(fid,'     }\n');
fprintf(fid,'\t return 0;\n');
fprintf(fid,'}\n');

fclose(fid);
end


% Helper functions 
function[] =  writePropensityString(fid,modelCumPropStrings,index)
fprintf(fid,['\t\t\t' modelCumPropStrings{index} '\n']);
fprintf(fid,'\t\t\tIS_PROPENSITY_NEGATIVE(%d, DRTB_CumProp[%d]);\n',index-1, index-1);
end



function[ret_val] = canBeWrittenToCurrentParallelSection(currentParallelSection,index,numberOfParallelSections)

ret_val = ( currentParallelSection < index ) && ...
          ( index <= ( currentParallelSection + 1) * numberOfParallelSections );
end
function[ret_val] = createNewParralelSection(currentParallelSection,numberOfPropensities,numberOfParallelSections)

ret_val = ( ( ( currentParallelSection + 2 ) * numberOfParallelSections )...
                                                     < numberOfPropensities );
end

function[] = writeEductString(fid,educt,modelStringMapping)

for i2 = 1:numel(educt),
        EductStr = dr_parseSymToString(educt(i2),modelStringMapping);
        fprintf(fid,[EductStr ' = ' EductStr ' - 1; ']);
end
end

function[] = writeProductString(fid,product,modelStringMapping)
for i2 = 1:numel(product),
    ProductStr = dr_parseSymToString(product(i2),modelStringMapping);
    fprintf(fid,[ProductStr ' = ' ProductStr ' + 1; ']);
end
end





