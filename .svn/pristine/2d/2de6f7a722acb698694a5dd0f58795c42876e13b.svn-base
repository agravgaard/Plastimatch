function [C,xm,ym,C_mag] = est_cov(score,range,verbose)
%% Range should be as (rows,cols).

if (~exist('verbose'))
  verbose = 1;
end

%% Need a couple more inputs here.
%% Sig expected and Sig min
%% sig_min = eye(2);
%% separate function for these??

%% Input is score
%[y,m] =  max(score(:));
%[r,c] = ind2sub(size(score),m);

%% Watershed doesn't work, because there may be local maxima
%% water = watershed(-score);
%% wlab = water(r,c);
%% dsp(water==wlab,1);
%% scn = score .* (score > 0) .* (water == wlab);

scn = score;
%% scn(score>y/2) = y/2;

%% Trim off edges
sz = size(score);
r = round((sz(1)-range(1))/2);
rows = r:r+range(1)-1;
c = round((sz(2)-range(2))/2);
cols = c:c+range(2)-1;
scn(1:20,:) = 0;
scn(sz(1)-20:sz(1),:) = 0;
scn(:,1:20) = 0;
scn(:,sz(2)-20:sz(2)) = 0;

[y,m] =  max(scn(:));
[r,c] = ind2sub(size(scn),m);

% bwl = bwlabel(score>0,4);
bwl = bwlabel(scn>0,4);
wlab = bwl(r,c);
scn = score .* (score > 0) .* (bwl == wlab);

% if (verbose)
% dsp(score,1);
% dsp(scn,1);
% end

C_mag = sum(sum(scn));
scn = scn / C_mag;

xm = sum([1:size(scn,2)].*sum(scn,1));
ym = sum([1:size(scn,1)]'.*sum(scn,2));

x1 = [1:size(scn,2)];
y1 = [1:size(scn,1)];
xs = ones(size(scn,1),1) * [1:size(scn,2)];
ys = [1:size(scn,1)]' * ones(1,size(scn,2));

%% 2d covariance about (xm,ym)
%% (the +0.5 is in case only 1 pixel wide)
sigx2 = 0.5 + sum(sum(scn,1) .* ([1:size(scn,2)] - xm).^2);
sigy2 = 0.5 + sum(sum(scn,2)' .* ([1:size(scn,1)] - ym).^2);
sigxy = sum(sum(scn .* (xs-xm) .* (ys-ym)));

C = [sigx2,  sigxy
     sigxy, sigy2    ];

sigx = sqrt(sigx2);
sigy = sqrt(sigy2);

if (verbose)
  ctr = [round(ym),round(xm)];
  rows = ctr(1)-10:ctr(1)+10;
  cols = ctr(2)-10:ctr(2)+10;
  dsp(i3(scn(rows,cols)),1,3);
  hold on;
  plot(11+xm-ctr(2),11+ym-ctr(1),'ro');
  D = [xs(:)-xm ys(:)-ym];
  %% tmp = sum((D.*(inv(C)*D')'),2);
  tmp = sum((D.*(inv(C)*D')'),2);
  tmp = reshape(tmp,size(scn));
  %% contour(tmp(21:40,21:40),20);
  %% uc = 4.41;
  uc = 1;
  contour(tmp(rows,cols),uc*[1 1]);
  hold on;
  drawnow;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
return;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% D = [xs(:)-xm ys(:)-ym];
D = [xs(:)-31 ys(:)-31];
tmp = sum((D.*(inv(C)*D')'),2);
tmp = reshape(tmp,size(scn));
contour(x1-31,y1-31,tmp,20);
hold on;
plot(0-sigx,0,'bx');
plot(0+sigx,0,'bx');
plot(0,0+sigy,'bx');
plot(0,0-sigy,'bx');
plot(0-2*sigx,0,'mx');
plot(0+2*sigx,0,'mx');
plot(0,0+2*sigy,'mx');
plot(0,0-2*sigy,'mx');
plot(0-3*sigx,0,'gx');
plot(0+3*sigx,0,'gx');
plot(0,0+3*sigy,'gx');
plot(0,0-3*sigy,'gx');
plot(0-4*sigx,0,'kx');
plot(0+4*sigx,0,'kx');
plot(0,0+4*sigy,'kx');
plot(0,0-4*sigy,'kx');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
return;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tmp = sum(scn,2) * sum(scn,1);
dsp(tmp,1);
hold on;
plot(xm,ym,'ro');
dsp(score,1);hold on;
plot(xm,ym,'ro');
