function mmd = marker_match(a,b)

xd = a(:,1)*ones(1,size(b,1))-ones(size(a,1),1)*b(:,1)';
yd = a(:,2)*ones(1,size(b,1))-ones(size(a,1),1)*b(:,2)';
dd = sqrt(xd.*xd + yd.*yd);
d1 = sort(min(dd));
d2 = sort(min(dd'));
if (size(dd,2) > 13)
  d1 = mean(d1(1:13));
  d2 = mean(d2(1:13));
else
  d1 = mean(d1(1:end));
  d2 = mean(d2(1:end));
end
mmd = min(d1,d2);
