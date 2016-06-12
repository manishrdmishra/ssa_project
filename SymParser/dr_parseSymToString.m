function [ outString ] = dr_parseSymToString(inSym, ModelStringMapping)
%DR_PARSESYMTOSTRING Helper: Map inSym to String according to mapping
%   This function is a very lazy helper function that, given a symbolic
%   Expression and a sym-to-string mapping returns the string the
%   Expression is mapped to. Here, warning/error handling could be
%   added...
	
    % Convert sym to char if input is sym
    if isa(inSym, 'sym'),
        outString = char(inSym);
    else
        outString = inSym;
    end
    
    % Replace generic State- and parameter names by CPP-Vector Name + Index
    for i2 = 1:numel(ModelStringMapping(:,1)),
        outString = strrep(outString, ModelStringMapping{i2,1}, ModelStringMapping{i2,2});
    end
        
end

