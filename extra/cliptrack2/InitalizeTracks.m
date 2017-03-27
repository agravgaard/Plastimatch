% =========================================================================
% function clipTracks = ...
%     InitalizeTracks(image, origImageSize, imageROI, template, nClips)
%
% Automatically initialize tracks based on the number of clips
%
% Input:
% 
% Output:
% 
% Author: Rui Li
% Date: 09/18/2009
% =========================================================================
function clipTracks = ...
    InitalizeTracks(subImage, imageROI, template, nClips)

% score image is indexed by nWidths * nTheta for different widths
% and rotation angles of the template
tSet = template.tSet;
patWidths = tSet.patWidths;
patHeight = tSet.patHeight;
patThetas = tSet.patThetas;
patCache = tSet.patCache;
nWidths = length(patWidths);
nThetas = length(patThetas);

scoreImage = TemplateMatch(subImage, patCache, nWidths, nThetas);

[maxScore, maxRotScaleIdx] = max(scoreImage, [], 3);

% locate the top n matches
simSize = size(scoreImage);
% clips: each row store information about [width height rotation r c]
clipTracks = zeros(nClips, 5);

for iClip=1:nClips
    [rIdx, cIdx] = ind2sub(simSize(1:2), find(maxScore==max(maxScore(:))));
    rotScaleIdx = maxRotScaleIdx(rIdx, cIdx);
    [iTheta, iWidth] = ind2sub([nThetas, nWidths], rotScaleIdx);
    bestTemplate = tSet.patCache{iWidth, iTheta, 1};
    [tHeight, tWidth] = size(bestTemplate);
    
    % set the all found tempalte area in the sortedScore to 0;
    maxScore(rIdx-(tHeight-1)/2 : rIdx+(tHeight-1)/2, ...
        cIdx-(tWidth-1)/2 : cIdx+(tWidth-1)/2) = 0;
    
    % clipTracks: each row store information about [length height rotation r c]
    clipTracks(iClip, :) = [patWidths(iWidth) patHeight ...
        patThetas(iTheta) rIdx+imageROI(2) cIdx+imageROI(1)];
end

