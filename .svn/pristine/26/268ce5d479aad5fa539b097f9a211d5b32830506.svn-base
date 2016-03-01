function [parts,vals] = fancc_quant_kmeans (A)

Au = A - mean(A(:));

A0 = sort(A(find(Au(:) < 0)));
A1 = sort(A(find(Au(:) > 0)));

part_01 = A0(floor(2*size(A0,1)/3));
part_12 = A1(floor(size(A1,1)/3));

A0 = A(A <= part_01);
A1 = A((A > part_01) & (A <= part_12));
A2 = A(A > part_12);

A0u = mean(A0(:));
A1u = mean(A1(:));
A2u = mean(A2(:));

%% This does three-way kmeans
if (0)
  A0u = mean(A0(:));
  A1u = mean(A1(:));
  A2u = mean(A2(:));

  centres = [A0u, A1u, A2u]';

  options(1) = 1;
  options(2) = .00000001;
  options(3) = .00000001;
  options(14) = 100;
  c_out = kmeans(centres, A(:), options);

  parts = [(c_out(2)+c_out(1)) / 2, (c_out(3)+c_out(2)) / 2 ];
end

A0 = A(A <= parts(1));
A1 = A((A > parts(1)) & (A <= parts(2)));
A2 = A(A > parts(2));

m = length(A0);
n = length(A2);
p = length(A1);

vv(2) = sqrt(m * (m + n + p) / (m*n + n*n));
vv(1) = - n * vv(2) / m;

vals = [ vv(1), 0, vv(2) ];
