function d = find_sq_distances (x, y)
%% x is a row vector, Each row of y is a measurement
%% Find the distance from x to all y's

hd1 = y - ones(size(y,1),1)*x;

d = sum(hd1.*hd1,2);
