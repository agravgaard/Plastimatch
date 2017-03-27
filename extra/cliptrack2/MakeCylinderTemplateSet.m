% =========================================================================
% function templateSet = MakeCylinderTemplateSet(width, height)
% 
% Make a set of cylinder templates based on the input width and height
% 
% Input:
%   width:  width of the template
%   height: height of the template
% 
% Output:
%   templateSet: a set of templates with different rotation angles
% 
% Author:   Rui Li
% 
% Date:     09/18/2009
% =========================================================================
function templateSet = MakeCylinderTemplateSet(winSize)

% template pattern height -- cylinder marker height (pixels)
% this is fixed because the marker height is almost negligible
patHeight = 2;

% template pattern width -- cylinder marker length (pixels)
minPatWidth = 6;
patWidthStep = 2;
maxPatWidth = 14;
% list of possible template pattern widthes
patWidths = minPatWidth:patWidthStep:maxPatWidth;
nPatWidths = length(patWidths);

% template angles
minPatTheta = 0;
patThetaStep = 5;
maxPatTheta = 180 - patThetaStep;  
patThetas = (minPatTheta:patThetaStep:maxPatTheta) * pi / 180;
nPatThetas = length(patThetas);

patRow = 0;
patCol = 0;
patMargin = 2;
fgColor = -1;
bgColor = 1;
weightWinHeight = patHeight + 5;
weightWinMargin = 3;

% allocate memory for the cell array that stores the pregenerated
% tempate pattern and weight maps
patCache = cell(nPatWidths, nPatThetas, 3);
for iPatWidth=1:nPatWidths
    for iPatTheta = 1:nPatThetas
        patParams = ...
            [patHeight, patWidths(iPatWidth), patThetas(iPatTheta), ...
            patRow, patCol, patMargin, fgColor, bgColor, ...
            weightWinHeight, weightWinMargin];
        [pat, weight] = MakeCylinderTemplate(winSize, patParams);
        patCache{iPatWidth, iPatTheta, 1} = pat;
        patCache{iPatWidth, iPatTheta, 2} = weight;
        patCache{iPatWidth, iPatTheta, 3} = (size(pat) - 1) / 2;
    end
end

templateSet.patWidthStep = patWidthStep;
templateSet.patWidths = patWidths;
templateSet.patHeight = patHeight;
templateSet.patThetaStep = patThetaStep;
templateSet.patThetas = patThetas;
templateSet.patCache = patCache;

