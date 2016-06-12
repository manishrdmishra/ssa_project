function [ModelStringMapping] = dr_prepParser(system)
%DR_PREPPARSER Create mapping of system to generic name for vars & paras
%   This function finds all symbolic state and variable names in the 
%   system and creates a cell array that maps the system specific
%   symbolic expressions to generic (c-compatible) variable names.
%
%   Input:
%       system is a cerena-style system struct
%
%   Output:
%       ModelStringMapping is nx2 cell array, where n is the total number
%       of variable and parameter names. ModelStringMapping(:,1) contains
%       the model specific name derived from the symbolic expression,
%       ModelStringMapping(:,2) the generic c compatible variable names.
%       The array is sorted by original string length (descending), to
%       avoid substring issues when using this mapping to find & replace
%       symbolic names.
%

    ModelStateStrings   = cell(numel(system.state.variable)    ,2);
    ModelParamStrings   = cell(numel(system.parameter.variable),2);
    StateStringLengths  = zeros(numel(system.state.variable)    ,1);
    ParamStringLengths  = zeros(numel(system.parameter.variable),1);
    for i = 1:numel(system.state.variable), 
        ModelStateStrings{i,1} = char(system.state.variable(i)); 
        ModelStateStrings{i,2} = ['DRTB_State[' num2str(i-1) ']'];
        StateStringLengths(i)  = numel(ModelStateStrings{i,1});
    end

    for i = 1:numel(system.parameter.variable),
        ModelParamStrings{i,1} = char(system.parameter.variable(i)); 
        ModelParamStrings{i,2} = ['DRTB_Param[' num2str(i-1) ']'];
        ParamStringLengths(i)  = numel(ModelParamStrings{i,1});
    end
    
    
    % To avoid problems when varnames are substrings of other varname
    % (e.g. var1 => X[0]; var1_inact => X[1]; parser replaces var1 first,
    % resulting in invalid var1_inact => X[0]_inact mapping), merge
    % parameter + state names and order according to size, replacing
    % largest names first, avoiding incorrect substring matches
    AllStringLengths    = [StateStringLengths; ParamStringLengths];
    ModelStringMapping  = [ModelStateStrings; ModelParamStrings];
    [~, newIndex]       = sort(AllStringLengths,1,'descend');
    ModelStringMapping  =  ModelStringMapping(newIndex,:);
end

