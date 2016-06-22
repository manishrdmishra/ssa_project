function [] = dr_compileModel(System, ExecID, cleanup)
%% Compile SSA-executable from Jan H.-style SSA System definition
%
% Input:
% System  - Jan H. style SSA system definition struct. Ask him for details.
%           (jan.hasenauer@helmholtz-muenchen.de)
% ExecID  - Name desired for the executable. Note that a temp folder 
%           ExecID_tmp will be created. Should this folder already exist, 
%           files might be overwritten. Confusion could commence. 
% cleanup - By default, temp files (c++ source) and folder will be deleted 
%           after compilation. By specifying cleanup = 0, you can tell the 
%           function to keep temp files, e.g. for debugging causes. This 
%           function is a very well behaved function and will almost 
%           certainly obey.
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
    % Set cleanup if not specified
    if ~exist('cleanup','var'),
        cleanup = 1;
    end
    
    % Create temporary working directory
    curDir = pwd;
    if ~exist([pwd() filesep() ExecID '_tmp'],'dir')
        mkdir([ExecID '_tmp']);
    end
    cd([ExecID '_tmp']);
 
    % Prepare sym -> string parser
    ModelStringMapping = dr_prepParser( System);

    % Write CPP files
    dr_writeModelDef(System,ModelStringMapping); % 'DRTB_modeldef_tmp.cpp'
    dr_writeModelDefHeader(System);              % 'DRTB_modeldefHeader_tmp.hpp'
    copyfile(which('DRTB_simulateSSA.cpp'),'DRTB_simulateSSA_tmp.cpp');
    copyfile(which('logger.hpp'),'logger.hpp');

    % Execute compilation
    % Aggressively optimized mex-call
    % Manish : removing -funroll-all-loops reduce the execution time
   

        mex  -output DRTB_executeable_tmp -v -cxx  ...
        CXXOPTIMFLAGS="-O3 -ffast-math " ...
        CXX='g++' ...
        CXXFLAGS="-std=c++11 -D_GNU_SOURCE -DLEVEL_ALL -DLOGGING  -fPIC -fno-omit-frame-pointer -fopenmp" ...
        LDOPTIMFLAGS=-O3 ...
        LDFLAGS='-pthread -shared -fopenmp -Wl,--no-undefined ' ...
        DRTB_simulateSSA_tmp.cpp DRTB_modeldef_tmp.cpp


%     mex  -output DRTB_executeable_tmp -v -cxx  ...
%         CXXOPTIMFLAGS="-O3 -ffast-math " ...
%         CXX='g++' ...
%         CXXFLAGS="-ansi -D_GNU_SOURCE -fPIC -fno-omit-frame-pointer -fopenmp" ...
%         LDOPTIMFLAGS=-O3 ...
%         LDFLAGS='-pthread -shared -fopenmp -Wl,--no-undefined ' ...
%         DRTB_simulateSSA_tmp.cpp DRTB_modeldef_tmp.cpp
    % Conservative, no c-optimization, no openMP mex-call
%     mex -output DRTB_executeable_tmp -v -cxx  ...
%         CXXOPTIMFLAGS="-O0 -funroll-all-loops" ...
%         CXXFLAGS="-ansi -D_GNU_SOURCE -fPIC -fno-omit-frame-pointer" ...
%         LDOPTIMFLAGS=-O0 ...
%         LDFLAGS='-pthread -shared -Wl,--no-undefined' ...
%         DRTB_simulateSSA_tmp.cpp DRTB_modeldef_tmp.cpp

    
    % Change back to start directory; move compiled file;
    % clean up tempfiles (if asked to)
    [~,~,MexExt] = fileparts(which('DRTB_executeable_tmp'));
    cd(curDir);
    movefile([ExecID '_tmp/DRTB_executeable_tmp' MexExt],[ExecID  MexExt]);
    if cleanup,
        rmdir([ExecID '_tmp'],'s');
    end
end

