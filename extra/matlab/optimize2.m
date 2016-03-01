%% Algorithm 3.27 in Madsen, Nielsen, and Tingleff
x0 = [1; 0];
delta = 1;
max_its = 90;
alg = 'bfgs';

alpha = 1;
B = eye (2);
x = x0;
[f,g] = rosenbrock (x);
disp (sprintf ('%10g [%10g %10g] [%10g %10g], %10g', ...
               f, x(1), x(2), g(1), g(2), alpha));
for it = 1:max_its
    
    %% Compute search direction
    h = (-B \ g);

    %% Compare with trust region
    if (norm(h) > delta)
        h = delta * h / norm(h);
    end
    
    %% Evaluate function
    x1 = x + h;
    [f1,g1] = rosenbrock (x1);

    %% Compute gain ratio with quadratic model
    gr = (f - f1) / (-h'*g + h'*B*h);
    if (gr < 0)
        delta = delta / 2;
    elseif (gr < 0.25)
        f = f1;g = g1;x = x1;
        delta = delta / 2;
    elseif (gr > 0.75)
        f = f1;g = g1;x = x1;
        delta = max(delta, 3 * norm(h));
    else
        f = f1;g = g1;x = x1;
    end

    %% Compute new B
    J = (sign(g) .* sqrt(2*abs(g)))';
    J1 = (sign(g1) .* sqrt(2*abs(g1)))';
    y = J1'*J1*h + (J1 - J)'*f1;
    hty = h' * y;
    if (hty > 0)
        v = B*h;
        B = B + (1/hty)*y*y' - (1/(h'*v))*v*v';
    end
    
    %% Go to next iteration
    disp (sprintf ('%10f [%10f %10f] [%10f %10f] %10f %10f',...
                   f, x1(1), x1(2), g(1), g(2), gr, delta));
end
