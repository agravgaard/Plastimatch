function [pat,wgt] = cyl_template(ws,pat_parms)
%% Same as markpat3, but size of outputs is trimmed

%% Now, with significantly more parameters!
pw = pat_parms(1);
pl = pat_parms(2);
pth = pat_parms(3);
pr = pat_parms(4);
pc = pat_parms(5);
pfa = pat_parms(6);
fgc = pat_parms(7);
bgc = pat_parms(8);
ww = pat_parms(9);
wfa = pat_parms(10);

px = ones(2*ws+1,1) * [-ws:ws];
py = flipud(px');
px = px - pc;
py = py + pr;

z = [px(:) py(:)];

v  = [ cos(pth),  sin(pth) ];   %% v is direction of theta
vp = [ -sin(pth), cos(pth) ];   %% vp is direction perp to theta

ls = v*z';                %% distance along v
ld = vp*z';               %% distance along vperp

%% Debug variables
ld1 = reshape(ld,2*ws+1,2*ws+1);
ls1 = reshape(ls,2*ws+1,2*ws+1);

%% pd = sqrt(sum(z.*z,2));   %% distance from origin

%% p1 and p2 are the two endpoints of line
p1 =  pl*ones(size(z,1),1)*v;
p2 = -pl*ones(size(z,1),1)*v;
pd1 = sqrt(sum((z-p1).*(z-p1),2));
pd2 = sqrt(sum((z-p2).*(z-p2),2));

%% Debug variables
qd1 = reshape(pd1,2*ws+1,2*ws+1);
qd2 = reshape(pd2,2*ws+1,2*ws+1);

%% Make distance map "td"
%% td = pd;
%% ldmask = abs(ls) < l;
%% td(ldmask) = abs(ld(ldmask));
%% td = reshape(td,2*ws+1,2*ws+1);
td = min(pd1,pd2);
ldmask = abs(ls) < pl;
td(ldmask) = abs(ld(ldmask));
td = reshape(td,2*ws+1,2*ws+1);

%% Debug variables
ldm1 = reshape(ldmask,2*ws+1,2*ws+1);

%% Make pattern
pat = - (td - (pw + pfa)) / pfa;
pat(pat>1) = 1;
pat(pat<0) = 0;

%% Make weight
wgt = - (td - (ww + wfa)) / wfa;
wgt(wgt>1) = 1;
wgt(wgt<0) = 0;

%% dsp(pat,1);hold on;contour(wgt);colormap jet;

pat_u = pat;
pat_w = fgc * pat + bgc * (1-pat);

pat = pat_w;

%% TRIM from 2*ws+1 to minimum necessary, but always centered at 
%% the origin.
cmin = min(find(sum(wgt)));
cmax = max(find(sum(wgt)));
rmin = min(find(sum(wgt')));
rmax = max(find(sum(wgt')));
cspread = max((ws+1) - cmin + 1, cmax - (ws+1) + 1);
rspread = max((ws+1) - rmin + 1, rmax - (ws+1) + 1);

%pat = pat((ws+1)-rspread:(ws+1)+rspread,(ws+1)-cspread:(ws+1)+cspread);
%wgt = wgt((ws+1)-rspread:(ws+1)+rspread,(ws+1)-cspread:(ws+1)+cspread);

if (rspread < ws)
  pat = pat((ws+1)-rspread:(ws+1)+rspread,:);
  wgt = wgt((ws+1)-rspread:(ws+1)+rspread,:);
end
if (cspread < ws)
  pat = pat(:,(ws+1)-cspread:(ws+1)+cspread);
  wgt = wgt(:,(ws+1)-cspread:(ws+1)+cspread);
end
