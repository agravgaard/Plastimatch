function template = circ_template(ws,cs)

template = - ones(2*ws+1,2*ws+1);
x = -ws:ws;
y = -ws:ws;
d = zeros(2*ws+1,2*ws+1);
d = d + ones(2*ws+1,1) * (x.*x);
d = d + (y.*y)' * ones(1,2*ws+1);
template(d<cs.*cs) = +1;
template = -template;
