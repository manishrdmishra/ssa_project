function [ModelCumPropStrings] = dr_parseCumProps(system,ModelStringMapping)
%DR_PARSECUMPROPS Parse symbolic props to c-code cumulative props
% 	Using the ModelStringMapping, this function parses all propensities 
% 	defined in the model into c-code expressions to calculate the vector of
% 	cumulative propensities . It depends on ccode being able to parse model 
% 	propensities and could use error checking.
    
    ModelCumPropStrings = cell(numel(system.reaction),1);
    for i = 1:numel(system.reaction), 
        ReactStringTemp = ccode(system.reaction(i).propensity);

        if i == 1
            ReactStringTemp = ['DRTB_CumProp[' num2str(i-1) '] =' ReactStringTemp((strfind(ReactStringTemp,'=')+1):end)];
        else
            ReactStringTemp = ['DRTB_CumProp[' num2str(i-1) '] = DRTB_CumProp[' num2str(i-2) '] +' ReactStringTemp((strfind(ReactStringTemp,'=')+1):end)];
        end

        ReactStringTemp = dr_parseSymToString(ReactStringTemp, ModelStringMapping);
        
        ModelCumPropStrings{i} = ReactStringTemp;
    end

end