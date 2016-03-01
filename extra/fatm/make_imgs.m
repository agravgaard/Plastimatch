close all;

B1 = double(readviv('C:\gsharp\idata\iris-fluoro\day1\0001\0_000446_0000002850.803.raw'));

B1win = [599 544 100 100];

B2 = double(readviv('C:\gsharp\idata\iris-fluoro\gating\0002\0_000514_0000094194.085.raw'));

B2win = [632,701,100,100];

B3 = double(readviv('C:\gsharp\idata\iris-fluoro\gating\0002\0_000774_0000094211.551.raw'));

B3win = [662,701,100,100];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

asz = 30;
asz = 50;
asz = 10;

A = ker(0.4*asz,asz)'*ker(0.4*asz,asz);
A = -A;
AW = ones(size(A));
awin = [1 1 size(A,1) size(A,2)];

B = B1;
bwin = B1win;
imno = 1;

B = B3;
bwin = B3win;
imno = 3;

B = B2;
bwin = B2win;
imno = 2;

%% score = mexwncc(A,AW,B,BW,awin,bwin,wthresh,sthresh);
global WNCC_A WNCC_AW WNCC_B WNCC_BW;
WNCC_A = A;
WNCC_AW = AW;
WNCC_B = B;
WNCC_BW = ones(size(WNCC_B));

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
%  figure;hold on;
%  plot(score2(:),score1(:),'b.');
end

clear functions;

tmp = score1(:)-score2(:)*std(A(:));
return;

%% 
IM = imnorm(B(bwin(1):bwin(1)+bwin(3),bwin(2):bwin(2)+bwin(4))) / 255 + 0.15;
imwrite (IM, sprintf('search_%d.tif',imno));
IM = imnorm(score1) / 255;
imwrite (IM, sprintf('score1_%d.tif',imno));
IM = imnorm(score2) / 255;
imwrite (IM, sprintf('score2_%d.tif',imno));
figure;hold on;plot(score1(41,:));plot(score2(41,:)*std(A(:)),'r');
print ('-depsc2',sprintf('plot_%d.eps',imno));
