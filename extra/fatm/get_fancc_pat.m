function QPAT = get_fancc_pat(A)
% close all;
addpath('C:/gsharp/projects/fmatch-2/source-3/Release/');

%% Pattern area
awin = [1 1 size(A,1) size(A,2)];

wthresh = awin(3) * awin(4) * 0.0001;
sthresh = 0.001;
bwin = [1 1 100 100];

[parts, vals] = fancc_quant (A);

options.command = 'compile';
options.alg = 'fancc';
options.pat_rect = awin;
options.sig_rect = bwin;
options.std_dev_threshold = sthresh;
options.wthresh = wthresh;
options.bin_partitions = parts;
options.bin_values = vals;
disp('Ready to compile');
[ptr,QPAT] = mexmatch(options,A);

options.command = 'free';
disp('Ready to free');
mexmatch(options,ptr);
% disp(sprintf('time0 = %g',t0/3));

clear functions;
