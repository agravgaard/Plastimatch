function mask = patient_mask(img)

T1 = -300;
T2 = -500;
T3 = -700;

im = mean(img,3);
imm = max(im);
clear im;

i1 = imm>T1;

v1 = min(find(i1));   %% Top of patient
v2 = max(find(i1));   %% Bot of couch
clear i1;

i2 = imm<T2;
i2(1:v1) = 0;
i2(v2:end) = 0;
u1 = min(find(i2));    %% Bot of patient
clear imm i2;

tmp = img < T3;
tmp(:,u1+1:end,:) = 1.0;

%% Fill holes in patient
[l,n] = bwlabeln(tmp,6);
%% Somtimes hist gives me out of memory errors, so use a loop instead
% a = hist(l(:),[0:n]);
for i=1:n+1
  disp(sprintf('Checking component %d/%d',i-1,n));
  a(i) = sum(l(:)==i-1);
end
[v,ind] = max([a(2:end)]);
mask1 = l ~= ind;

%% Remove speckles outside patient
[l,n] = bwlabeln(mask1,6);
a = hist(l(:),[0:n]);
[v,ind] = max([a(2:end)]);
mask = (l == ind);
