%% Conjugate gradient with soft line search.  Simple heuristic on setting b.
%% See Eqn 4.11 
%%   in Frandsen, Jonasson, Nielsen, and Tingleff's booklet:
%%   "Unconstrained Optimization" 
%%   http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/3217/pdf/imm3217.pdf
%% See also: http://www2.imm.dtu.dk/~hbn/immoptibox/
%x0 = [1; 0];
x0 = [-1.2; 1];
delta = 1;
max_its = 1;
alg = 'conjugate-gradient-2';

alpha = 1;
x = x0;
[f,g] = rosenbrock (x);
disp (sprintf ('%10f [%10f %10f] [%10f %10f], %d %10f', f, x(1), x(2), g(1), g(2), 0, alpha));
gamma = 0;
hcg = [0;0];
%alpha_prev = 0.5 / (g'*g);
%alpha_prev = 0.5 / sqrt(g'*g);
%alpha_prev = 3.0 / sqrt(g'*g);
alpha_prev = 0.5;
hist = [];
trace = [];
for it = 1:max_its
    
    %% Compute search direction
    hprev = hcg;
    hcg = -g + gamma*hprev;

    %% Check for wrong direction
    if (g' * hcg > 0)
        hcg = -hcg;
    end
    
    %% Initialize line search
    %a = 0; b = alpha_prev * 2;
    a = 0; b = 1;
    amax = 10;
    %rho = 0.2; beta = 0.5;
    %%rho = 1e-2; beta = 0.1;       % Used by example 4.3
    %%rho = 1e-3; beta = 0.99;      % Used by immoptbox?
    %rho = 1e-3; beta = 0.5;
    %rho = 1e-3; beta = 0.5;
    %rho = 1e-3; beta = 0.1;
    rho = 1e-3; beta = 1e-3;
    phi_a = f;
    phip_a = g' * hcg;
    lambda_a = f;
    phip_0 = phip_a;
    gamma_ls = beta * phip_0;      % gamma in alg 2.27
    k = 1; kmax = 10;

    %% Evaluate function
    x1 = x + b * hcg;
    [f1,g1] = rosenbrock (x1);
    hist = [hist, [f1;x1]];
    trace=[trace, b];
    phi_b = f1;
    phip_b = g1' * hcg;
    lambda_b = f + rho * phip_0 * b;
    feval = 1;
    %disp (sprintf ('%10f [%10f %10f] [%10f %10f], %d %10f', f1, x1(1), x1(2), g1(1), g1(2), 0, b));
    
    %% Perform line search part 1
    while (phi_b < lambda_b && phip_b < gamma_ls ...
           && b < amax && k < kmax)

        %% Refine b
        a = b;
        b = b * 2;
        phi_a = phi_b;
        phip_a = phip_b;
        lambda_a = lambda_b;

        %% Evaluate function
        x1 = x + b * hcg;
        [f1,g1] = rosenbrock (x1);
        hist = [hist, [f1;x1]];
        trace=[trace, b];
        phi_b = f1;
        phip_b = g1' * hcg;
        lambda_b = f + rho * phip_0 * b;
        feval = feval + 1;
        %disp (sprintf ('%10f [%10f %10f] [%10f %10f], %d %10f', f1, x1(1), x1(2), g1(1), g1(2), 1, b));
        k = k + 1;
    end

    %% Perform line search part 2
    alpha = b;
    phi_alpha = phi_b;
    phip_alpha = phip_b;
    lambda_alpha = lambda_b;
    k = 1; kmax = 10;
    while ((phi_alpha > lambda_alpha || phip_alpha < gamma_ls) && k < kmax)

        %% Refine alpha, a, b
        d = b - a;
        c = (phi_b - phi_a - d*phip_a) / (d*d);
        C = (phi_b - phi_a - d*phip_a)
        [phi_b phi_a d phip_a]
        c
        if (c > 0)
            alpha = a - phip_a / (2*c)
            alpha = min(max(alpha, a+0.1*d), b-0.1*d);
        else
            alpha = (a + b) / 2;
        end

        %% Evaluate function
        x1 = x + alpha * hcg;
        [f1,g1] = rosenbrock (x1);
        trace=[trace, alpha];
        hist = [hist, [f1;x1]];
        phi_alpha = f1;
        phip_alpha = g1' * hcg;
        lambda_alpha = f + rho * phip_0 * b;
        feval = feval + 1;
        %disp (sprintf ('%10f [%10f %10f] [%10f %10f], %d %10f', f1, x1(1), x1(2), g1(1), g1(2), 2, alpha));
        
        
        [phi_alpha lambda_alpha]
        if (phi_alpha < lambda_alpha)
            a = alpha;
            phi_a = phi_alpha;
            phip_a = phip_alpha;
            lambda_a = lambda_alpha;
        else
            b = alpha;
            phi_b = phi_alpha;
            phip_b = phip_alpha;
            lambda_b = lambda_alpha;
        end 
        [a,alpha,b]
        
        
        k = k + 1;
        [phi_alpha, lambda_alpha, phip_alpha, gamma_ls]
    end

    alpha_prev = alpha;
    gprev = g;
    f = f1;g = g1;x = x1;

    disp (sprintf ('%10f [%5f %5f] [%5f %5f], %d', f, x(1), x(2), g(1), g(2), feval));
    
    %% Fletcher-Reeves
    gamma = (g' * f) / (gprev' * gprev);
        

    %% Polak-Ribiere
    gamma = ((g-gprev)' * g) / (gprev' * gprev);
end

plot(hist(2,:),hist(3,:),'r*');axis([0 1.5 0 1.5]);
