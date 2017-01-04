%% Load model from file
% The system is contained in the file ImmuneSystem.mat
    load ImmuneSystem

%% SETUP simulation parameters
% Parameter vector
    timeScale = 24*3600;
    theta = [   100000                   % N_0
                100000                   % V_init
                0.001                    % scale_act               % (a) Activation
                0.047   * 24/timeScale   % P_egr
                0.5                      % dP_egr_cortex -> alpha
                0.8                      % dP_egr_medulla -> beta  % (b) Egress to blood
                0.012   * 24/timeScale   % P_migr_med -> P_medulla % (c) Migration to medulla
                0.05    * 24/timeScale   % P_egr_tis               % (d) Loss from blood to tissue
                0.11    * 24/timeScale   % P_prol
                0.5                      % r_prol -> theta
                10000                    % scale_V                 % (f) Proliferation   11
                0.01    * 24/timeScale   % P_Ki67_down             % (g) Ki67 downregulation
                0.02    * 24/timeScale   % P_death                 % (h) Cell death
                0.002   * 24/timeScale   % P_Bcl2_downB
                0.03    * 24/timeScale   % P_Bcl2_down             % (i) Bcl2 downregulation
                0.008   * 24/timeScale   % P_cxcr4_down            % (j) CXCR4 downregulation
                5                        % dP_CD62L_down_medulla -> mu
                0.001   * 24/timeScale   % P_CD62L_down            % (k) CD62L downregulation
                0.2     * 24/timeScale   % rho_V                   % (l) Virus production
                6.25e-7 * 24/timeScale]; % delta_V    


% Initial conditions (all 0 except x0(4) = x__01101_cortex_0, x0(97) = V_0)
    x0 = zeros(97,1);
    x0(14) = 1e5;
    x0(97) = 1e5;
    Nssa = 1;
    options.scale = 'absolute';

% Time vector
% Timescale of model is days/timeScale; e.g. for timeScale=1, 
%    t = linspace(0,50*timeScale,1000); % full 50 day simulation
    t = linspace(0,2*timeScale,1000); % short 1 day test simulation
    
    %% Finish system (cerena toolbox)
    System = completeSystem(System);
    System = completeSystemSSA(System);
    
    %% Compile executable
    
     num_of_threads = 2;
    %% specify the compiler options
    % To get details about these flags, please look at dr_compileModel.m file
    
   
    compiler_options.cleanup = 0;
    compiler_options.optimization = 1;
    compiler_options.logging = 0;
    compiler_options.logging_level = 2;
    compiler_options.num_of_threads = num_of_threads;
    execName = 'largeExample01_basic';
    dr_compileModel(System, execName, compiler_options);
    %   copyfile(which(execName),[fileparts(which('run_largeEx01')) filesep 'bin' filesep]);
    %   delete([execName '.*']);
    
    %% specify the options for mex executable
    
    % To get details about these flags, please look at dr_runSSAWithModel.m file
    
    program_options.panic_file_name = 'panic_log.txt';
    program_options.periodic_file_name = 'periodic_log.txt';
    program_options.max_history = cast(100,'uint64');
    program_options.period = cast(100,'uint64');
    program_options.num_of_threads = cast(num_of_threads,'uint64');
    
    %% Run simulation
    %   modelName = 'largeExample01_optimized';
    tic
    modelName = 'largeExample01_basic';
    timecourse = dr_runSSAWithModel(t, x0, theta, program_options, modelName,Nssa);
    fprintf('C-based simulation time:');
    toc


    
