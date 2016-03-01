addpath('C:/gsharp/projects/cliptrack/');

asz = 10;
% asz = 25;

A05 = ker(0.05*asz,asz)'*ker(0.05*asz,asz);
A1 = ker(0.1*asz,asz)'*ker(0.1*asz,asz);
A15 = ker(0.15*asz,asz)'*ker(0.15*asz,asz);
A2 = ker(0.2*asz,asz)'*ker(0.2*asz,asz);
A25 = ker(0.25*asz,asz)'*ker(0.25*asz,asz);
A3 = ker(0.3*asz,asz)'*ker(0.3*asz,asz);
A4 = ker(0.4*asz,asz)'*ker(0.4*asz,asz);
A8 = ker(0.8*asz,asz)'*ker(0.8*asz,asz);

a2 = 1.0;
w = A8;
w = A8 - max(A8(1,:));
w(w < 0) = 0;
A = w .* (A4 - a2 * A2);

% parms = cyl_default_parms();
% A = parms.template_library.template{1,25,1}.*parms.template_library.template{1,25,2};

close all
dsp(A,1);
figure;hold on;
plot(A(:,1),'r');
plot(A(:,asz+1),'b');

[parts,vals] = fancc_quant_kmeans (A);

QPAT = get_fancc_pat(A);

dsp(QPAT,1);

Atest = ones(size(A))*vals(3);
Atest(A <= parts(2)) = vals(2);
Atest(A <= parts(1)) = vals(1);

dsp(Atest,1);

Aref = A - mean(A(:));
Aref = Aref / std(Aref(:));

figure; hold on;
plot(Aref(asz+1,:));
plot(Atest(asz+1,:),'g.-');
plot(QPAT(asz+1,:),'r--');

tmp = imnorm(Aref) / 255;
imwrite(tmp, 'template_ref.tif');
tmp = imnorm(QPAT) / 255;
imwrite(tmp, 'template_fast.tif');
tmp = imnorm(Atest) / 255;
imwrite(tmp, 'template_kmeans.tif');
