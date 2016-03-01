
% Scanner geometry
slice_size_mm = 256;
slice_dim = 128;

% Phantom geometry
phantom_diameter = 200;
plug_diameter = 30;
plug_position = 55;

% HU values of inserts
plugs = [
    -1000
    100
    35
    -200
    700
    -100
];

a = -1000 * ones(slice_dim,slice_dim);
spacing = slice_size_mm / slice_dim;
offset = - (slice_size_mm - spacing) / 2;

Ainfo.ElementSpacing = [spacing spacing 5];
Ainfo.Offset = [offset offset -20];

x = Ainfo.Offset(1) + [0:size(a,1)-1]*Ainfo.ElementSpacing(1);
x = ones(size(a,2),1) * x;
y = Ainfo.Offset(2) + [0:size(a,2)-1]*Ainfo.ElementSpacing(2);
y = y' * ones(1,size(a,1));

d2 = x.*x + y.*y;
a(d2 < (phantom_diameter/2)^2) = 0;

for i=1:length(plugs)
    angle = (i-1) * 2*pi/length(plugs)
    plug_center = [
        sin(angle) * plug_position
        cos(angle) * plug_position
    ];
    d2 = (x-plug_center(1)).^2 + (y-plug_center(2)).^2;
    a(d2 < (plug_diameter/2).^2) = plugs(i);
end


A = zeros(slice_dim,slice_dim,9);
for i=1:9
    A(:,:,i) = a;
end

writemha ('ct_phantom.mha', A, Ainfo.Offset, Ainfo.ElementSpacing, ...
          'short');
