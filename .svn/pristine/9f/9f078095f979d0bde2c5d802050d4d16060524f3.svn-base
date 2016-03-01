% n = number of points
n = 10;
%n = 20;
%n = 80;

%phi = (sqrt(5)+1)/2 - 1;
phi = 3 - sqrt(5);
ga = phi * pi;

a = [];
for i = 1:n
    longitude = mod (ga*i, 2*pi);
    latitude = asin(-1 + 2*i/n);
    a = [a; longitude latitude];
end

x = [];
for i = 1:n
    x1 = cos (a(i,2)) * cos (a(i,1));
    x2 = cos (a(i,2)) * sin (a(i,1));
    x3 = sin (a(i,2));
    x = [x; x1, x2, x3];
end
