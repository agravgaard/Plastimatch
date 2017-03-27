function [circle] = makesphere(r,xsize,ysize,pixel_size)
% Generates a sphere in a 3D matrix
%
% r = radius of sphere; we will later find how to calculate this
% ysize = the size of the y-grid that will contain the sphere
% zsize = the size of the z-grid that will contain the sphere
% pixel_size = pixel_size, in cm

if (nargin < 4),
    error ('Please see help for INPUT DATA.');
end

s=zeros(xsize,ysize);
xdim = 0.5*(xsize - 1)*pixel_size;
ydim = 0.5*(ysize - 1)*pixel_size;
[x,y]=meshgrid(-xdim:pixel_size:xdim, -ydim:pixel_size:ydim);  % creates x,y grids to 
d2 = x.^2+y.^2;
circle = sqrt(d2) < r;
