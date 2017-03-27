% =========================================================================
% function DrawTrackedTemplate(nTemplates, templateParams)
% 
% Draw the templates on the source image based on the estestimated
% parameters like location, scale and rotation
% 
% Author: Rui Li
% Date: 09/17/2009
% =========================================================================
function DrawTrackedTemplate(nTemplates, templateParams)
gcf; hold on;
colorSpecs = ['y','m','c','r','g','b','w','k'];
for iTemplate = 1:nTemplates
    % the size and rotation angle of the template
    tLength = templateParams(iTemplate, 1);
    tHeight = templateParams(iTemplate, 2);
    tAngle = templateParams(iTemplate,3);
    
    % center in the source image coordinate system
    tRow = templateParams(iTemplate,4);
    tCol = templateParams(iTemplate,5);
    
    DrawRectangle(tLength, tHeight, tAngle, tRow, tCol, ...
        colorSpecs(iTemplate));
end

hold off;

