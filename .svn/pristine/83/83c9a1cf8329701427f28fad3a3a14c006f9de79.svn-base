function dsp(A,normalize,figno)
% Display the matrix as an image
% 
% dsp(A,normalize,figno)
% 
% A is the matrix, normalize is 1 or 0 (to normalize or not), and figno is
% the figure number.

if (nargin < 1)
  disp ('missing required input argument "A"');
  disp ('Usage: dsp(A,normalize);');
  return;
end
if (nargin < 2)
  normalize = 0;
end
if (nargin < 3)
  figure;
end

A = double(A);
if (normalize)
  if (ndims(A)==2)
    A = A - min(min(A));
    A = 255 * A / max(max(A));
  else
    A = A - min(min(min(A)));
    A = A / max(max(max(A)));
  end
else
  A = abs(A);
end

if (nargin >= 3 && ~isempty(figno))
  figure(figno);
  clf;
end
image(A);

if (ndims(A)==2)
  colormap(gray(256));
end
