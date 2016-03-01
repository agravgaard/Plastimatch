function Z = prefilt(A)

smoothing = 0;

sid = 1630;
sad = 1000;
% psz = 50/15;
psz = .192 * 2;
pso = psz * sad / sid;
%% psz == pixel size

[x,y] = meshgrid(1:size(A,2),1:size(A,1));

x = x - (size(A,2)+1) / 2;
y = y - (size(A,1)+1) / 2;
x = x .* psz;
y = y .* psz;
d = sqrt((x.^2 + y.^2 + sid^2));
Aw = sid ./ d;

%% 2d smoothing kernel

if (smoothing)
  ws = 5;
  ks = 1.5;
  k = ker2 (ks,ws);
  A = conv2(A,k,'same');
end

%% Weighting
A2 = A .* Aw;

%% 1d hpf kernel
ks = 20;
kd = -ks:ks;
warning off MATLAB:divideByZero;
kr = - 1 ./ (2*kd.^2*pi^2*pso.^2);
warning on MATLAB:divideByZero;
kr(1:2:end) = 0;
kr(ks+1) = 1 / (8*pso^2);

kr = kr / sum(kr);

Z = conv2(A2,kr,'same');

%% Hack to get rid of edge artifacts
Z(:,2) = Z(:,3);
Z(:,1) = Z(:,3);
Z(:,end) = Z(:,end-2);
Z(:,end) = Z(:,end-1);
