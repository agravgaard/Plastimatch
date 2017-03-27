function [X Y Z] = makesphere2(center, radius, NOP)
%---------------------------------------------------------------------------------------------
% [X Y] = circle(center, radius, NOP)
% This routine draws a circle with center defined as
% a vector CENTER, radius as a scaler RADIS. NOP is 
% the number of points on the circle.
%
%   Usage Examples,
%
%   [X Y] = circle([1, 3], 3, 1000);
%   [X Y] = circle([2, 4], 2, 1000);
%   plot(X, Y);
%   axis equal;
%
%   Zhenhai Wang <zhenhai@ieee.org>
%   Version 1.00
%   December, 2002
%   
%   Modified by Alan Chu
%---------------------------------------------------------------------------------------------

if (nargin < 3),
 error('Please see help for INPUT DATA.');
elseif (nargin == 3)
    style = 'b-';
end;
THETA = linspace(0, 2*pi, NOP);
R = ones(1, NOP)*radius;
PHI = linspace(0,2*pi,NOP);
[X Y Z] = sph2cart(THETA, PHI, R);
X = X + center(1);
Y = Y + center(2);
Z = Z + center(3);
plot3(X, Y, Z, style);
axis square;
grid on;