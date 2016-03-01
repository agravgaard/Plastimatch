%[a,ainfo] = readmha ('hi_gcs.mha');

%wx = 5 * ones(size(a));
%wy = zeros(size(a));
%wz = zeros(size(a));

res = [50,50,50];
offset = [-25,-25,-25];
spacing = [2,2,2];

w(:,:,:,1) = 5 * ones(res);
w(:,:,:,2) = zeros(res);
w(:,:,:,3) = zeros(res);
writemha ('synth_x_5mm_vf.mha',w,offset,spacing,'float');

w(:,:,:,1) = zeros(res);
w(:,:,:,2) = 5 * ones(res);
w(:,:,:,3) = zeros(res);
writemha ('synth_y_5mm_vf.mha',w,offset,spacing,'float');

w(:,:,:,1) = zeros(res);
w(:,:,:,2) = zeros(res);
w(:,:,:,3) = 5 * ones(res);
writemha ('synth_z_5mm_vf.mha',w,offset,spacing,'float');

[x,y] = meshgrid([1:res(1)]+1/2-res(1)/2,[1:res(2)]+1/2-res(2)/2);
d = sin(20*atan2(x,y));
xd = x / res(1);
yd = y / res(2);
wx = (10 * d .* xd)';
for i=1:size(w,3)
  w(:,:,i,1) = wx;
end
wy = (10 * d .* yd)';
for i=1:size(w,3)
  w(:,:,i,2) = wy;
end
w(:,:,:,3) = zeros(res);
writemha ('synth_radial_vf.mha',w,offset,spacing,'float');
