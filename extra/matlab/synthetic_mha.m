function A=synthetic_mha(fn,type,options)
% function synthetic_mha(fn,type,options)
%
% Type: either rect, gaussian, or sphere
% 
%  General Options
%    size      size of volume in mm
%    nvox      number of voxels in volume
%    spacing   pixel spacing in mm
%    fg        foreground pixel value
%    bg        background pixel value
%
%  Rect options
%    extent  size of foreground rectangle in voxels
%
%  Sphere options
%    radius  size of sphere in voxels
%    center  center of sphere in voxels
%
%  Gaussian options
%    std     width of gaussian in voxels
%    center  center of sphere in voxels
%
% Example usage:
%  fn = 'synthetic_1.mha';
%  type = 'gaussian';
%  options.nvox = 128;
%  options.center = [64.5,64.5,64.5];
%  synthetic_mha(fn,type,options);
% 
%  fn = 'synthetic_2.mha';
%  type = 'gaussian';
%  options.nvox = 128;
%  options.center = [64.5,54.5,64.5];
%  synthetic_mha(fn,type,options);
% 
%  fn = 'synthetic_3.mha';
%  type = 'rect';
%  options.nvox = 100;
%  options.extent = [40,60;40,60;40,60];
%  synthetic_mha(fn,type,options);
% 
%  fn = 'synthetic_4.mha';
%  type = 'rect';
%  options.nvox = 100;
%  options.extent = [42,62;40,60;40,60];
%  synthetic_mha(fn,type,options);

if (isfield(options,'size'))
  vsize = options.size;
else
  vsize = 500;
end
if (length(vsize)==1)
  vsize = [vsize, vsize, vsize];
end

if (isfield(options,'nvox'))
  nvox = options.nvox;
else
  nvox = 64;
end
if (length(nvox)==1)
  nvox = [nvox, nvox, nvox];
end

if (isfield(options,'spacing'))
  spacing = options.spacing;
else
  spacing = vsize ./ nvox;
end
if (length(spacing)==1)
  spacing = [spacing, spacing, spacing];
end

if (isfield(options,'offset'))
  offset = options.offset;
else
  offset = -spacing .* (nvox/2 - 0.5);
end
if (length(offset)==1)
  offset = [offset, offset, offset];
end
if (isfield(options,'fg'))
  fg = options.fg;
else
  fg = 1000;
end
if (isfield(options,'bg'))
  bg = options.bg;
else
  bg = -1000;
end

A = bg * ones(nvox(1),nvox(2),nvox(3));

switch lower(type)
 case 'rect'
  if (isfield(options,'extent'))
    ext = options.extent;
  else
    ext = [10,nvox-10;10,nvox-10;10,nvox-10];
  end
  A(ext(1,1):ext(1,2),ext(2,1):ext(2,2),ext(3,1):ext(3,2)) = fg;

 case 'sphere'
  if (isfield(options,'center'))
    ctr = options.center;
  else
    ctr = [nvox/2,nvox/2,nvox/2] + 0.5;
  end
  if (isfield(options,'radius'))
    rad = options.radius;
  else
    rad = 10;
  end
  [ax,ay,az] = ndgrid(1-ctr(1):nvox-ctr(1),...
		      1-ctr(2):nvox-ctr(2),...
		      1-ctr(3):nvox-ctr(3));
  d = ax.^2 + ay.^2 + az.^2;
  A(d<rad^2) = fg;

 case 'gaussian'
  if (isfield(options,'center'))
    ctr = options.center;
  else
    ctr = [nvox/2,nvox/2,nvox/2] + 0.5;
  end
  if (isfield(options,'std'))
    coeff = options.std;
  else
    coeff = nvox/4;
  end
  
  [ax,ay,az] = ndgrid((1-ctr(1):nvox-ctr(1))/coeff(1),...
		      (1-ctr(2):nvox-ctr(2))/coeff(2),...
		      (1-ctr(3):nvox-ctr(3))/coeff(3));
  d = ax.^2 + ay.^2 + az.^2;
  v = [exp(-d/2)];
  v = v / max(v(:));
  A = -1000 + (fg+1000) * v;
end

writemha (fn, A, offset, spacing, 'short');
