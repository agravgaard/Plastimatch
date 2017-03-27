function k2 = ker2(coeff,kx)
%% k2 = ker2(coeff,kx)
%%   kx is the number of x coordinates
%%   coeff is the std deviation (normally chosen approx 1/3 - 1/4 of kx)

if (isscalar(kx))
  kx = -kx:kx;
end
k=exp((-kx.^2)/(2*coeff^2));

k2 = k'*k;
k2=k2 / sum(k2(:));
