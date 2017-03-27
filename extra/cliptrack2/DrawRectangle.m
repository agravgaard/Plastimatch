% rLength, rHeight: the long and short sides of the rectangle
% rAngle: the angle of rotation of the rectangle
% x0, y0: the center of the rectangle
% rColor: the color of the lines of the rectangle
function DrawRectangle(rLength, rHeight, rAngle, y0, x0, rColor, lineWidth)

if nargin < 7
    lineWidth = 2;
end
% add some margin to the tight template width and height
% for better visualization
xMargin = 8; 
yMargin = 4;
rLength = rLength + xMargin;
rHeight = rHeight + yMargin;

% the 4 corners of the rectangle
xn = [-rLength rLength rLength -rLength];
yn = [-rHeight -rHeight rHeight rHeight];

% rotation matrix
rot = [cos(rAngle), sin(rAngle); ...
    -sin(rAngle), cos(rAngle)];

% rotate rectangle
xy = rot * [xn; yn];

x = xy(1,:) + x0;
y = xy(2,:) + y0;
plot(x([1 2 3 4 1]),y([1 2 3 4 1]),[rColor,'-'], 'LineWidth', lineWidth);


