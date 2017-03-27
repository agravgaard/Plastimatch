function out = make_real(x)
% out = make_real(x)
%
% x is a vector. out is the same vector except with 0 replacing imaginary
% elements.

out = x;
for k = 1:length(x)
    if ~isreal(x(k))
        out(k) = 0;
    end
end
