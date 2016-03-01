%% Conjugate gradient with soft line search.  Use immoptibox for line search.
%% Ref: Frandsen, Jonasson, Nielsen, and Tingleff's booklet:
%%   "Unconstrained Optimization" 
%%   http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/3217/pdf/imm3217.pdf
%% See also: http://www2.imm.dtu.dk/~hbn/immoptibox/
%x0 = [1; 0];
x0 = [-1.2; 1];
max_its = 1;
alg = 'conjugate-gradient-3';

alpha = 1;
x = x0;
[f,g] = rosenbrock (x0);
disp (sprintf ('%10g [%10g %10g] [%10g %10g], %d %10g', f, x(1), x(2), g(1), g(2), 0, alpha));
gamma = 0;
hcg = [0;0];
hist = [];
for it = 1:max_its
    
    %% Compute search direction
    hprev = hcg;
    hcg = -g + gamma*hprev;
    %disp (sprintf ('     hprev: %4f %4f, hcg: %4f %4f, gamma: %9f',hprev(1), hprev(2), hcg(1), hcg(2), gamma));

    %% Check for wrong direction
    if (g' * hcg > 0)
        hcg = -hcg;
    end
    
    %% Evaluate line search
    [x1,f1,g1,info,trace] = linesearch(@rosenbrock,x,f,g,hcg,[]);
    disp (sprintf ('%10f [%5f %5f] [%5f %5f], %d', ...
                   f1, x1(1), x1(2), g1(1), g1(2), info(3)));
    hist = [hist, [f1;x1]];
    
    gprev = g;
    f = f1;g = g1;x = x1;

    %% Fletcher-Reeves
    gamma = (g' * f) / (gprev' * gprev);
        
    %% Polak-Ribiere
    gamma = ((g-gprev)' * g) / (gprev' * gprev);

end

plot(hist(2,:),hist(3,:),'r*');axis([0 1.5 0 1.5]);
