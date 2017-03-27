% =========================================================================
% function scoreImage = FastTemplateMatch(I, T)
%
% Compute normalized cross correlation using FFT, speed up with the
% the use of integral image
% 
% Input:
%   I: input image or sub image
%   tSet: template library
% 
% Output:
%   The normalized cross correlation. The values range between 0 and 1
% =========================================================================
function scoreImage = TemplateMatch(I, patCache, nWidths, nThetas)

% initialize score image
iSize = size(I);
scoreImage = zeros([iSize(1) iSize(2) nWidths*nThetas]);

idx = 1;
for iWidth = 1:nWidths
    for iTheta = 1:nThetas
        template = patCache{iWidth, iTheta, 1};
        tSize = size(template);
        scoreImage(:,:,idx) = FastTemplateMatch(double(I), double(template),...
            iSize, tSize);
        idx  = idx + 1;
    end
end








