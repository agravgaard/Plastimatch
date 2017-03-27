% =========================================================================
% function state = GenerateInitialParticleSet(initialTrack, ...
%     patWidths, patThetas, nClips, nParticles)
% 
% Author:   Rui Li
% Date:     09/17/2009
% =========================================================================
function [state, linkLengths] = GenerateInitialParticleSet(initialTrack, ...
    patWidths, patThetas, nClips, nParticles)

% % total number of possible template size and orientation
% nWidths = length(patWidths);
% nThetas = length(patThetas);

% linkLengths store the pair wise Euclidean distance between all clips
clipPos = initialTrack(:, end-1:end);
% trick to get indices
[i,j] = find(triu(ones(nClips), 1));
% initialize output matrix
linkLengths = zeros(nClips, nClips);
linkLengths( i + nClips*(j-1) ) = ...
    sqrt(sum(abs( clipPos(i,:) - clipPos(j,:) ).^2, 2));
linkLengths( j + nClips*(i-1) ) = linkLengths( i + nClips*(j-1) );

state = cell(nClips, 1);
for iClip = 1:nClips
    % the height of the clips do not change
    % lengths for the clips
    iWidth = find(patWidths == initialTrack(iClip, 1));
    
    % rotation angles for the clips
    iTheta = find(patThetas == initialTrack(iClip, 3));
    
    % nParticles: number of particles per configuration (width, theta);
    particles = mvnrnd(initialTrack(iClip, end-1:end), diag([16; 49])*eye(2), nParticles);
    particles = unique(int32(particles), 'rows');
    nParticles = size(particles, 1);
    
    % samples, each row stores [width indexes, theta indexes,
    %                           clip postion Y, clip position X]
    samples = [iWidth(ones(1,nParticles),:), ...
               iTheta(ones(1,nParticles),:), ...
               particles(:,1), particles(:,2)];

    state{iClip} = samples;
end


