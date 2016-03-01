function err = checkgrad (fn, x, eps)
    
[f,g] = eval ([fn, '([', num2str(x), '])']);

x1 = x + eps * [1, 0];
[f1,g1] = eval ([fn, '([', num2str(x1), '])']);

x2 = x + eps * [0, 1];
[f2,g2] = eval ([fn, '([', num2str(x2), '])']);

g

df = ([f1-f, f2-f] ./ eps)

err = 1;
