%% SYSTEM / SETUP
% (R1) DNA_off -> DNA_on  , tau_on*DNA_off
% (R2) DNA_on  -> DNA_off , tau_off*DNA_on
% (R3)       0 -> mRNA    , k_m*DNA_on
% (R4)    mRNA -> 0       , gamma_m*mRNA
% (R5)       0 -> Protein , k_p*mRNA
% (R6) Protein -> 0       , gamma_p*Protein
% (R7) Protein + DNA_off -> Protein + DNA_on , tau_on_p*Protein*DNA_off
% colFSP = [0.5,0.5,0.5;  ...
%           0.6,0.6,0.6;  ...
%           0.9,0.9,0.9];
% col_zc = [0.0,0.0,0.7];
clear
% Parameter vector
theta1 = [1.00;1.00;10;1;4;1;0.015];
theta2 = [0.2;0.2;10;1;4;1;0.015];

% theta = theta1;
theta = theta2;

% Initial condition
xi = 0.3;
lambdaRoff = 4;
lambdaRon  = 4;
lambdaPoff = 10;
lambdaPon  = 10;

% Time vector
t = [linspace(0,20,100)];

%% MODEL
% Definition of symbolic variables:
syms DNA_off DNA_on mRNA Protein;
syms tau_on tau_off k_m gamma_m k_p gamma_p tau_on_p;
syms time

% In new CERENA system Volume is defined as 
% symbolic variable. 
syms Omega % volume

% Define state vector:
system.time = time;
system.state.variable = [ DNA_off    ;  DNA_on    ; mRNA       ; Protein ];
system.state.number   = length(system.state.variable);
system.state.type     = {'stochastic';'stochastic';'moment'    ;'moment' };
system.state.name     = {'DNA_{off}' ; 'DNA_{on}' ;'mRNA'      ;'Protein'};
system.state.xmin     = [      0     ;      0     ;    0       ;    0    ];
system.state.xmax     = [      1     ;      1     ;   inf      ;   inf   ];
system.state.mu0      = [      1     ;      0     ;    4       ;    10   ];
system.state.C0       = zeros(system.state.number*(system.state.number+1)/2,1);
system.state.constraint = @(X) ((X(1)+X(2)) == 1);
% Define parameter vector:
system.parameter.variable = [ tau_on     ; tau_off     ; k_m ; gamma_m   ; k_p ; gamma_p   ; tau_on_p];
system.parameter.name     = {'\\tau_{on}';'\\tau_{off}';'k_m';'\\gamma_m';'k_p';'\\gamma_p';'\\tau_{on,p}'};
% Define propensities:
% (R1)
system.reaction(1).educt      = DNA_off;
system.reaction(1).product    = DNA_on;
system.reaction(1).propensity = tau_on*DNA_off;
%system.reaction(1).propensity = tau_on;
system.reaction(1).parameter  = tau_on;
% (R2)
system.reaction(2).educt      = DNA_on;
system.reaction(2).product    = DNA_off;
system.reaction(2).propensity = tau_off*DNA_on;
system.reaction(2).parameter  = tau_off;
% (R3)
system.reaction(3).educt      = DNA_on;
system.reaction(3).product    = [mRNA,DNA_on];
system.reaction(3).propensity = k_m*DNA_on;
system.reaction(3).parameter  = k_m;
% (R4)
system.reaction(4).educt      = mRNA;
system.reaction(4).product    = [];
system.reaction(4).propensity = gamma_m*mRNA;
system.reaction(4).parameter  = gamma_m;
% (R5)
system.reaction(5).educt      = mRNA;
system.reaction(5).product    = [Protein,mRNA];
system.reaction(5).propensity = k_p*mRNA;
system.reaction(5).parameter  = k_p;
% (R6)
system.reaction(6).educt      = Protein;
system.reaction(6).product    = [];
system.reaction(6).propensity = gamma_p*Protein;
system.reaction(6).parameter  = gamma_p;
% (R7)
system.reaction(7).educt      = [DNA_off,Protein];
system.reaction(7).product    = [DNA_on ,Protein];
system.reaction(7).propensity = tau_on_p*Protein*DNA_off;
system.reaction(7).parameter  = tau_on_p;

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
xmin_FSP = [0;0; 0;  0];
xmax_FSP = [1;1;40;150];
[system] = getFSP(system,xmin_FSP,xmax_FSP);

% Initial condition
model.p0 = zeros(size(system.index,1),1);
for ix = 1:size(system.index,1)
    if system.index(ix,1) == 0
        model.p0(ix) =     xi*poisspdf(system.index(ix,3),lambdaRoff)*poisspdf(system.index(ix,4),lambdaPoff);
    else
        model.p0(ix) = (1-xi)*poisspdf(system.index(ix,3),lambdaRon )*poisspdf(system.index(ix,4),lambdaPon );
    end
end




%% specify the compiler options
% To get details about these flags, please look at dr_compileModel.m file

compiler_options.cleanup = 0;
compiler_options.optimization = 1;
compiler_options.logging = 1;
compiler_options.logging_level = 0;

dr_compileModel(system,'testAtefeh',compiler_options);



% Drawing of steady state
a  = find(cumsum(model.p0)>=rand,1,'first');


%% specify the options for mex executable 

% To get details about these flags, please look at dr_runSSAWithModel.m file

program_options.panic_file_name = 'panic_log.txt';
program_options.periodic_file_name = 'periodic_log.txt';
program_options.max_history = cast(100,'uint64');
program_options.period = cast(1,'uint64');

x0 = system.index(a,:)';

% write all the input parameters for mexFunction to .mat file
writeInputToMatFile('input.mat',t, x0, theta, program_options);



