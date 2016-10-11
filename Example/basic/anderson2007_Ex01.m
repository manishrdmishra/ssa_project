%% SYSTEM / SETUP
% (R1) X1 + X2  ->  X3  , c1*X1*X2
% (R2) X3  -> 0 , c2*X3

clear
% Parameter vector
theta1 = [1.00;1.00];
theta2 = [0.001;0.001];

% theta = theta1;
theta = theta2;

% Initial condition
% xi = 0.3;
% lambdaX1 = 4;
% lambdaX2  = 4;
% lambdaC3 = 10;


% Time vector
t = [linspace(0,20,100)];

%% MODEL
% Definition of symbolic variables:
syms X1 X2 X3;
syms c1 c2;
syms time

% In new CERENA system Volume is defined as
% symbolic variable.
syms Omega % volume

% Define state vector:
system.time = time;
system.state.variable = [X1    ;  X2    ; X3 ];
system.state.number   = length(system.state.variable);
system.state.type     = {'stochastic';'stochastic';'moment'};
system.state.name     = {'X1' ; 'X2' ;'X2'};
system.state.xmin     = [      0     ;      0     ;    0    ];
system.state.xmax     = [      1000    ;      1000     ; 2000   ];
system.state.mu0      = [      1     ;      0     ;    4    ];
system.state.C0       = zeros(system.state.number*(system.state.number+1)/2,1);
system.state.constraint = @(X) ((X(1)+X(2)) == 2000);
% Define parameter vector:
system.parameter.variable = [ c1 ; c2 ];
system.parameter.name     = {'\\c1';'\\c2'};
% Define propensities:
% (R1)
system.reaction(1).educt      = X1;
system.reaction(1).educt      = X2;
system.reaction(1).product    = X3;
system.reaction(1).propensity = c1*X1*X2;
system.reaction(1).parameter  = c1;
% (R2)
system.reaction(2).educt      = X3;
system.reaction(2).product    = [];
system.reaction(2).propensity = c2*X3;
system.reaction(2).parameter  = c2;


% Compartments
system.compartments = {'default'};
system.volumes = Omega;
for i = 1:length(system.state.variable)
    system.state.compartment{i} = 'default';
    system.state.volume(i)      = system.volumes;
end

%% SIMULATION OF FSP
% Construction
system = completeSystem(system);
xmin_FSP = [0;0; 0;];
xmax_FSP = [1;1;40];
[system] = getFSP(system,xmin_FSP,xmax_FSP);

%Nssa = 1000;
Nssa = 2;

%% Optimized SSA simulation
tic,

%% specify the compiler options
% To get details about these flags, please look at dr_compileModel.m file

compiler_options.cleanup = 0;
compiler_options.optimization = 1;
compiler_options.logging = 1;
compiler_options.logging_level = 0;

dr_compileModel(system,'testAtefeh',compiler_options);

tic,

% Drawing of steady state
%a  = find(cumsum(model.p0)>=rand,1,'first');


%% specify the options for mex executable

% To get details about these flags, please look at dr_runSSAWithModel.m file

program_options.panic_file_name = 'panic_log.txt';
program_options.periodic_file_name = 'periodic_log.txt';
program_options.max_history = cast(100,'uint64');
program_options.period = cast(1,'uint64');

%x0 = system.index(a,:)';
x0 = [1000;1000;0];
dr_X_SSA = dr_runSSAWithModel(t,x0,theta,program_options,'testAtefeh',Nssa); % This function returns the state vector X_SSA at the prespecified time
% vector t.

dr_m_SSA = mean(dr_X_SSA,3);
dr_C_SSA = var(dr_X_SSA,[],3);
fprintf('C-based simulation time:');

toc,

figure(1);title('C-based simulation mean + var')
subplot(4,2, 1);plot(t,dr_m_SSA(:,:))
subplot(4,2, 2);plot(t,dr_C_SSA(:,:))
subplot(4,2, 3);plot(t,dr_m_SSA(:,1))
subplot(4,2, 4);plot(t,dr_C_SSA(:,1))
subplot(4,2, 5);plot(t,dr_m_SSA(:,2))
subplot(4,2, 6);plot(t,dr_C_SSA(:,2))
subplot(4,2, 7);plot(t,dr_m_SSA(:,3))
subplot(4,2, 8);plot(t,dr_C_SSA(:,3))
