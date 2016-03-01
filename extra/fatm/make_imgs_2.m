addpath('C:/gsharp/projects/cliptrack/');

idx = 63;
parms = cyl_default_parms();
A = parms.template_library.template{1,idx,1}.*parms.template_library.template{1,idx,2};

close all
dsp(A,1);

[parts,vals] = fancc_quant (A);

[parts,vals] = fancc_quant_kmeans (A);

QPAT = get_fancc_pat(A);

dsp(QPAT,1);

Atest = ones(size(A))*vals(3);
Atest(A <= parts(2)) = vals(2);
Atest(A <= parts(1)) = vals(1);

dsp(Atest,1);

Aref = A - mean(A(:));
Aref = Aref / std(Aref(:));

tmp = imnorm(Aref) / 255;
imwrite(tmp, 'clip_template_ref.tif');
tmp = imnorm(QPAT) / 255;
imwrite(tmp, 'clip_template_fast.tif');
tmp = imnorm(Atest) / 255;
imwrite(tmp, 'clip_template_kmeans.tif');
