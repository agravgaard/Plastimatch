function f = sphere_match_obj (x, A, Ainfo)

com = x(1:3);
radius = x(4);
B = makesphere (Ainfo, com, radius);
dice = 2 * sum(B(:).*A(:)) / (sum(B(:)) + sum(A(:)));
printf ('[%g %g %g %g] %g\n', com(1), com(2), com(3), radius, dice);
f = -dice;
