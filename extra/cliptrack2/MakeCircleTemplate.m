% =========================================================================
% function pat = MakeCylinderTemplate(patWinSize, radius)
%  
% generate a template and weights for cylindrical markers
% 
% Input:
%   patWinSize: size of the template window
%   radius: radius of the circle
% 
% Output:
%   pat: the generated pattern (image) for the marker
% 
% Author:
%   Rui Li
% Date:
%   09/18/2009
% =========================================================================
function pat = MakeCircleTemplate(patWinSize, radius)

pat = - ones(2*patWinSize+1,2*patWinSize+1);
x = -patWinSize:patWinSize;
y = -patWinSize:patWinSize;
d = zeros(2*patWinSize+1,2*patWinSize+1);
d = d + ones(2*patWinSize+1,1) * (x.*x);
d = d + (y.*y)' * ones(1,2*patWinSize+1);
pat(d < radius.*radius) = pat(d < radius.*radius) + 1;
pat = -pat;