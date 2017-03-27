m = 4;
p = [0.2 0.3 0.2 0.3 ];
cum_weight = cumsum(p);
samp = rand(m,1);
bin1 = zeros(m,1);

for i=1:m
    ind = find(cum_weight >= samp(i));
    bin1(ind(1)) = bin1(ind(1)) + 1;
end

bin2 = zeros(m,1);
for i=1:m-1
    bin2 = bin2 + (cum_weight >= samp(i));
end



