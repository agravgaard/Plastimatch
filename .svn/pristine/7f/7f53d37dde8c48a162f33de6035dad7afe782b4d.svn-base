%% This combines a steepest descent direction with trust interval line search.
%% See Eqn 2.8 + Eqn 2.20 in Madsen, Nielsen, and Tingleff's 
%%   booklet: "Methods for non-linear least squares probelms"
%%   http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/3215/pdf/imm3215.pdf
%% See also: http://www2.imm.dtu.dk/~hbn/immoptibox/
x0 = [1;0];
max_its = 90;
alg = 'steepest';

alpha = 1;
x = x0;
[f,g] = rosenbrock (x);
disp (sprintf ('%10g [%10g %10g] [%10g %10g], %10g', ...
               f, x(1), x(2), g(1), g(2), alpha));
for it = 1:max_its
    
    %% Compute search direction
    h = -alpha * (g / norm(g));
    
    %% Evaluate function
    x1 = x + h;
    [f1,g1] = rosenbrock (x1);

    %% Compute gain ratio with linear model
    gr = (f - f1) / (-h'*g);
    if (gr < 0)
        alpha = alpha / 2;
    elseif (gr < 0.25)
        f = f1;g = g1;x = x1;
        alpha = alpha / 2;
    elseif (gr > 0.75)
        f = f1;g = g1;x = x1;
        alpha = alpha * 3;
    else
        f = f1;g = g1;x = x1;
    end
    
    %% Go to next iteration
    disp (sprintf ('%10f [%10f %10f] [%10f %10f] %10f %10f', ...
                   f, x1(1), x1(2), g(1), g(2), alpha, gr));
end
