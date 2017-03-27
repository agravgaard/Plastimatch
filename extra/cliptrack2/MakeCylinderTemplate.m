% =========================================================================
% function [pat, weight] = MakeCylinderTemplate(ws, patParams)
%  
% generate a template and weights for cylindrical markers
% 
% Input:
%   ws: size of the template window
%   patParams: parameters for the template pattern and its associated
%       weight map
% 
% Output:
%   pat: the generated patter (image) for the marker
%   weight: the associated weight map
% 
% Author:
%   Rui Li
% Date:
%   09/18/2009
% =========================================================================
function [pat, weight] = MakeCylinderTemplate(patWinSize, patParams)

% get the parameters for generating the template pattern and associated
% weight map
patHeight = patParams(1);
patWidth = patParams(2);
patTheta = patParams(3);
patRow = patParams(4);
patCol = patParams(5);
patMargin = patParams(6);
fgColor = patParams(7);
bgColor = patParams(8);
wWinHeight = patParams(9);
wWinMargin = patParams(10);

% template center is at (0,0), in template coordinate system
patX = ones(2*patWinSize + 1, 1) * [-patWinSize:patWinSize];
patY = flipud(patX');
% if patRow and patCol (center of the template pattern) are not zeros, 
% we need to do the coordinate system translation
patX = patX - patCol;
patY = patY - patRow;

% z is the matrix hold all the x y coordinates of the template pattern
z = [patX(:), patY(:)];

% v is the direction of theta
v = [cos(patTheta) sin(patTheta)];
% vp is perpendicular to v
vp = [-sin(patTheta), cos(patTheta)];

% all the pixel distance to the axis along v directionof the template
ls = v * z';
% all the pixel distance to the axis along vp directionof the template 
ld = vp * z';

% p1 and p2 are the two endpoints along the longest axis of the pattern
tmp = patWidth * ones(length(patX(:)), 1);
p1 = tmp * v;
p2 = -tmp * v;
% pd1 and pd2 are the distance maps based on the two end points p1, p2
pd1 = sqrt(sum((z-p1) .* (z-p1), 2));
pd2 = sqrt(sum((z-p2) .* (z-p2), 2));
td = min(pd1, pd2);
td(abs(ls) < patWidth) = abs(ld(abs(ls) < patWidth));
td = reshape(td, 2*patWinSize+1, 2*patWinSize+1);

% make the template pattern based on the foreground color (fgColor)
% and background color (bgColor)
pat = -(td - (patHeight + patMargin)) / patMargin;
% pat(pat > 1) = 1;
% pat(pat < 0) = 0;
pat = fgColor * pat + bgColor * (1 - pat);

% make weight map, basically it gives more importance to
% the pixels that are closer to the central axis
weight = -(td - (wWinHeight + wWinMargin)) / wWinMargin;
weight(weight > 1) = 1;
weight(weight < 0) = 0;

% trim the tempalte size, does not always have to be 2 * patWinSize + 1
minCol = find(sum(weight), 1, 'first');
maxCol = find(sum(weight), 1, 'last');
minRow = find(sum(weight, 2), 1, 'first');
maxRow = find(sum(weight, 2), 1, 'last');

colSpread = max((patWinSize+1)-minCol+1, maxCol-(patWinSize+1)+1);
rowSpread = max((patWinSize+1)-minRow+1, maxRow-(patWinSize+1)+1);

if rowSpread < patWinSize
    pat = pat((patWinSize+1)-rowSpread:(patWinSize+1)+rowSpread, :);
    weight = weight((patWinSize+1)-rowSpread:(patWinSize+1)+rowSpread, :);
end

if colSpread < patWinSize
    pat = pat(:,(patWinSize+1)-colSpread:(patWinSize+1)+colSpread);
    weight = weight(:,(patWinSize+1)-colSpread:(patWinSize+1)+colSpread);
end
