% =========================================================================
% function template = InitCylinderTemplate()
%  
% generate a default set of templates for cylindrical markers
% 
% Input:
%   NONE
% 
% Output:
%   The template struct which stores the default parameters and
%   template sets
% 
% Author:
%   Rui Li
% Date:
%   09/18/2009
% =========================================================================
function template = InitCylinderTemplate()

% default template window size
template.tWinSize = 23;

% default search window size
template.sWinSize = 70;


% set template type
template.type = 'cylinder';


% generate a set of templates with different angles
tSet = MakeCylinderTemplateSet(template.tWinSize);
template.tSet = tSet;