a = load ('signal-reg1d.txt');

sigma = 1.5;
g = exp(-((-4:.1:4).^2)/2/sigma);
g = g / sum(g);
t = conv(a,g);
t2 = t(length(g):end-length(g)-1);
hw = (length(g)+1)/2;
t2 = [ones(hw,1)*t2(1);t2;ones(hw,1)*t2(end)];

a_adj = a - t2;

av = a(2:end) - a(1:end-1);
av = [av(1); av];

phase = atan2 (a_adj, av);

