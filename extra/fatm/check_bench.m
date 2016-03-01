a = loadpfm('../src/pat.pfm');
b = loadpfm('../src/sig.pfm');
c = loadpfm('../src/sco.pfm');
d = loadpfm('../src/qpat.pfm');

c1 = normxcorr2(a,b);
c1 = c1(size(a,1):size(c1,1)-size(a,1)+1,size(a,2):size(c1,2)-size(a,2)+1);

close all;
figure(1); clf; hold on;
plot(c(100,:),'b');
plot(c1(100,:),'r');

dsp(a,1);
dsp(d,1);

dsp(c,1);
dsp(c1,1);
