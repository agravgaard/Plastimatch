%% Conjugate gradient with trust interval line search.
%% See Eqn 4.11 
%%   in Frandsen, Jonasson, Nielsen, and Tingleff's booklet:
%%   "Unconstrained Optimization" 
%%   http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/3217/pdf/imm3217.pdf
x0 = [1; 0];
delta = 1;
max_its = 80;
alg = 'conjugate-gradient-1';

alpha = 1;
x = x0;
[f,g] = rosenbrock (x);
disp (sprintf ('%10g [%10g %10g] [%10g %10g], %10g', ...
               f, x(1), x(2), g(1), g(2), alpha));
gamma = 0;
hcg = [0;0];
hist = [];

for it = 1:max_its
    
    %% Compute search direction
    hprev = hcg;
    hcg = -g + gamma*hprev;

    %% Perform line search
    line_search_complete = 0;
    while ~line_search_complete

        %% Compare with trust region
        if (norm(hcg) > delta)
            hcg = delta * hcg / norm(hcg);
        end

        %% Check for wrong direction
        if (g' * hcg > 0)
            hcg = -hcg;
        end

        %% Evaluate function
        x1 = x + hcg;
        [f1,g1] = rosenbrock (x1);
        hist = [hist, [f1;x1]];

        %% Compute gain ratio with linear model
        gr = (f - f1) / (-hcg'*g);
        if (gr < 0)
            delta = delta / 2;
        elseif (gr < 0.25)
            delta = delta / 2;
            line_search_complete = 1;
        elseif (gr > 0.75)
            delta = max(delta, 3 * norm(hcg));
            line_search_complete = 1;
        else
            line_search_complete = 1;
        end

        disp (sprintf ('%9f %d [%9f %9f] [%9f %9f] %9f %9f',...
                       f1, line_search_complete, ...
                       x1(1), x1(2), g(1), g(2), gr, delta));
    end

    if (~line_search_complete)
        disp (sprintf ('Error: line search failed'));
    end

    gprev = g;
    f = f1;g = g1;x = x1;

    %% Fletcher-Reeves
    gamma = (g' * f) / (gprev' * gprev);
        
    %% Polak-Ribiere
    gamma = ((g-gprev)' * g) / (gprev' * gprev);
        
end
