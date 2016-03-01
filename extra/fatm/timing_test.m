close all;
addpath('C:/gsharp/projects/fmatch-2/source-3/Release/');
% addpath('C:/gsharp/projects/fmatch-2/source-2/Release/');

B = double(readviv('C:\gsharp\idata\iris-fluoro\day1\0001\0_000446_0000002850.803.raw'));
BW = ones(size(B));

%% Format is: [rmin,cmin,nrow,ncol]
%% And don't forget C index starts with 0!

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

asz = 50;
asz = 30;
asz = 5;
asz = 10;
asz = 20;

A = ker(0.4*asz,asz)'*ker(0.4*asz,asz);
AW = ones(size(A));

%% Pattern area
awin = [1 1 size(A,1) size(A,2)];

%% Search area
bwin = [-20 1 100 100];
bwin = [-20 2001 200 200];
bwin = [1 1 1 1];
bwin = [1 1 100 100];
bwin = [1 1 250 250];
bwin = [1 1 350 350];
bwin = [1 1 450 450];
bwin = [1 1 600 600];
bwin = [1 1 size(B,1) size(B,2)];
bwin = [1 1 800 800];
bwin = [601 551 200 200];
bwin = [101 101 72 72];
bwin = [1 1 72 72];
bwin = [1 1 100 100];
bwin = [1 1 250 250];

%% score = mexwncc(A,AW,B,BW,awin,bwin,wthresh,sthresh);
global WNCC_A WNCC_AW WNCC_B WNCC_BW;
WNCC_A = A;
WNCC_AW = AW;
WNCC_B = B;
WNCC_BW = BW;
wthresh = awin(3) * awin(4) * 0.0001;
sthresh = 0.001;

options.command = 'timing_test';
options.alg = 'ncc';
options.alg = 'fncc';
options.alg = 'fancc';
options.pat_rect = awin;
options.sig_rect = bwin;
options.std_dev_threshold = sthresh;
options.wthresh = wthresh;
[parts, vals] = fancc_quant (WNCC_A,'fast');
options.bin_partitions = parts;
options.bin_values = vals;
timing = mexmatch(options,WNCC_A,WNCC_B);
timing

clear functions;
