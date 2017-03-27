% =========================================================================
% function template = InitCircleTemplate()
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
function template = InitCircleTemplate()

% default template window size
template.tWinSize = 9;

% default search window size
template.sWinSize = 70;

% circle radius
template.radius = 7.6;

% set template type
template.type = 'circle';

% generate a template, for circular template, the size
% is fixed and it is rotation invariant
template.tSet = MakeCircleTemplate(template.tWinSize, template.radius);
