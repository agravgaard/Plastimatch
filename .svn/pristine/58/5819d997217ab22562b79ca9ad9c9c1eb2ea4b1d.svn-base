close all;
% addpath('C:/gsharp/projects/fmatch-2/source-3/Release/');
% addpath('C:/gsharp/projects/fmatch-2/source-2/Release/');
addpath('C:/gsharp/libs/fatm-vc6/RelWithDebInfo');

rand('state',0);

A = rand(1000) * 255;
AW = ones(1000);
%% AW = double(rand(1000) > 0.7);

B = rand(1000) * 255;
BW = ones(1000);
%% BW = double(rand(1000) > 0.7);

if (0)
options.alg = 'ncc';
options.command = 'run';
options.pat_rect = [700 550 30 30];
options.sig_rect = [700 150 100 100];
options.std_dev_threshold = 0.001;
options.wthresh = 0.001;

s = mexmatch(options, A, AW, B, BW);
clear functions;
end


%% B = double(readviv('C:\gsharp\idata\iris-fluoro\day1\0001\0_000446_0000002850.803.raw'));

rand('state',0);
B = 100 * ker2(3,500) + rand(1001);

B1 = B;

%% Format is: [rmin,cmin,nrow,ncol]
%% And don't forget C index starts with 0!

if (0)
%% Symetric test
awin = [700 550 30 30];
bwin = [700 150 100 100];

%% Assymetric test 2
awin = [700 550 30 30];
bwin = [700 150 110 100];

%% Assymetric test 3
awin = [700 550 40 30];
bwin = [700 150 100 120];

%% Assymetric test 1
awin = [700 550 40 30];
bwin = [700 150 100 100];

%% Small window
awin = [200 550 50 50];
bwin = [100 150 70 70];

%% Large window
awin = [200 550 150 200];
bwin = [100 150 250 250];
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

asz = 30;
asz = 50;
asz = 10;

% A = ker(0.4*asz,asz)'*ker(0.4*asz,asz);
% A = -A;

A = ker2(0.4*asz,asz);

AW = ones(size(A));

%% Pattern area
awin = [1 1 size(A,1) size(A,2)];

%% Search area
bwin = [-20 1 100 100];
bwin = [-20 2001 200 200];
bwin = [1 1 250 250];
bwin = [1 1 350 350];
bwin = [1 1 450 450];
bwin = [1 1 600 600];
bwin = [1 1 800 800];
bwin = [1 1 size(B,1) size(B,2)];
bwin = [1 1 100 100];
bwin = [1 1 1 1];
bwin = [601 551 100 100];

%% score = mexwncc(A,AW,B,BW,awin,bwin,wthresh,sthresh);
global WNCC_A WNCC_AW WNCC_B WNCC_BW;
WNCC_A = A;
WNCC_AW = AW;
WNCC_B = B;
WNCC_BW = BW;

%% Compare matlab implementation with C implementation

alg1 = 'mexwncc_v1';
alg1 = 'mexfncc';
alg1 = 'normxcorr2';
alg1 = 'mexfancc';
alg1 = 'mexmatch_fancc';
alg1 = 'mexncc';
alg1 = 'mexmatch_ncc';
alg1 = 'mexmatch_fncc';

alg2 = 'normxcorr2';
alg2 = 'mexncc';
alg2 = 'mexfancc';
alg2 = 'mexmatch_ncc';
alg2 = 'mexfncc';
alg2 = 'mexmatch_fncc';
alg2 = 'mexmatch_fancc';

score1 = mncc(awin,bwin,alg1); t1 = 0;
score2 = mncc(awin,bwin,alg2); t2 = 0;
tic; score1 = mncc(awin,bwin,alg1); t1 = t1 + toc;
tic; score2 = mncc(awin,bwin,alg2); t2 = t2 + toc;
% tic; score1 = mncc(awin,bwin,alg1); t1 = t1 + toc;
% tic; score2 = mncc(awin,bwin,alg2); t2 = t2 + toc;
% tic; score1 = mncc(awin,bwin,alg1); t1 = t1 + toc;
% tic; score2 = mncc(awin,bwin,alg2); t2 = t2 + toc;
disp(sprintf('time %20s = %g',alg1,t1/3));
disp(sprintf('time %20s = %g',alg2,t2/3));

if (isempty(find(score2-score1)))
  disp('Perfect match');
elseif (max(abs(score2(:)-score1(:))) < 1e-13)
  disp('Near match');
else
  dsp(score1,1);
  dsp(score2,1);
  dsp(score2-score1,1);
  d = score2 - score1;
  figure;hold on;
  plot(score2(1:100:end),score1(1:100:end),'b.');
end

clear functions;
