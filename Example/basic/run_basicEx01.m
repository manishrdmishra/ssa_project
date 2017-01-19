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


%Nssa = 1000;
Nssa = 1;
%% Matlab based SSA simulation
% v = @(X) theta.*[X(1);X(2);X(2);X(3);X(3);X(4);X(1)*X(4)];
% S = [-1 +1  0  0  0  0 -1;...
%     +1 -1  0  0  0  0  1;...
%     0  0 +1 -1  0  0  0;...
%     0  0  0  0 +1 -1  0];
% tic,
% gs_X_SSA = zeros(length(t),4,Nssa);
% for i = 1:Nssa
%     % Drawing of steady state
%     a  = find(cumsum(model.p0)>=rand,1,'first');
%     x0 = system.index(a,:)';
%     gs_X_SSA(:,:,i) = simulateSSA(v,S,t,x0)'; % This function returns the state vector X_SSA at the prespecified time
%     % vector t.
% end
% gs_m_SSA = mean(gs_X_SSA,3);
% gs_C_SSA = var(gs_X_SSA,[],3);
% fprintf('Original simulation time:');
% toc,

%% Optimized SSA simulation
tic,

 num_of_threads = 1;

%% specify the compiler options

cleanup = 0;
optimization = 1;
logging = 1;
logging_level = 2;

% To get details about these flags, please look at dr_compileModel.m file

compiler_options.cleanup = cast(cleanup,'uint8');
compiler_options.optimization = cast(optimization,'uint8');
compiler_options.logging = cast(logging,'uint8');
compiler_options.logging_level = cast(logging_level,'uint8');
compiler_options.num_of_threads = cast(num_of_threads,'uint8');

dr_compileModel(system,'testAtefeh',compiler_options);

tic,

% Drawing of steady state
a  = find(cumsum(model.p0)>=rand,1,'first');


%% specify the options for mex executable 


% To get details about these flags, please look at dr_runSSAWithModel.m file

program_options.panic_file_name = 'panic_log.txt';
program_options.periodic_file_name = 'periodic_log.txt';
program_options.max_history = cast(100,'uint64');
program_options.period = cast(10,'uint64');
program_options.num_of_threads = cast(num_of_threads,'uint64');
x0 = system.index(a,:)';
%x0 = [1000;1000;100;50];
dr_X_SSA = dr_runSSAWithModel(t,x0,theta,program_options,'testAtefeh',Nssa); % This function returns the state vector X_SSA at the prespecified time

% vector t.
%dr_X_SSA = dr_runSSA(system,t, x0, theta, program_options, Nssa);
dr_m_SSA = mean(dr_X_SSA,3);
dr_C_SSA = var(dr_X_SSA,[],3);
fprintf('C-based simulation time:');

toc,

%% Visualize results
% figure(1);title('Original simulation mean + var')
% subplot(5,2, 1);plot(t,gs_m_SSA(:,:))
% subplot(5,2, 2);plot(t,gs_C_SSA(:,:))
% subplot(5,2, 3);plot(t,gs_m_SSA(:,1))
% subplot(5,2, 4);plot(t,gs_C_SSA(:,1))
% subplot(5,2, 5);plot(t,gs_m_SSA(:,2))
% subplot(5,2, 6);plot(t,gs_C_SSA(:,2))
% subplot(5,2, 7);plot(t,gs_m_SSA(:,3))
% subplot(5,2, 8);plot(t,gs_C_SSA(:,3))
% subplot(5,2, 9);plot(t,gs_m_SSA(:,4))
% subplot(5,2,10);plot(t,gs_C_SSA(:,4))

figure(2);title('C-based simulation mean + var')
subplot(5,2, 1);plot(t,dr_m_SSA(:,:))
subplot(5,2, 2);plot(t,dr_C_SSA(:,:))
subplot(5,2, 3);plot(t,dr_m_SSA(:,1))
subplot(5,2, 4);plot(t,dr_C_SSA(:,1))
subplot(5,2, 5);plot(t,dr_m_SSA(:,2))
subplot(5,2, 6);plot(t,dr_C_SSA(:,2))
subplot(5,2, 7);plot(t,dr_m_SSA(:,3))
subplot(5,2, 8);plot(t,dr_C_SSA(:,3))
subplot(5,2, 9);plot(t,dr_m_SSA(:,4))
subplot(5,2,10);plot(t,dr_C_SSA(:,4))