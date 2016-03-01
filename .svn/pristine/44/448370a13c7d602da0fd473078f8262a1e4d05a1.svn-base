function final_hypos = match_circle (parms, rowcol, Btmp, BWtmp)

global WNCC_A WNCC_AW WNCC_B WNCC_BW;

ws = parms.ws;
ss = parms.ss;

WNCC_A = parms.template;
WNCC_AW = ones(size(WNCC_A));
WNCC_B = Btmp;
WNCC_BW = BWtmp;
psz = floor(size(WNCC_A) / 2);

awin = [1,1,size(WNCC_A,1),size(WNCC_A,2)];
padj = [ws,ws] - psz;
bwin = [padj(1)+1,padj(2)+1,...
	size(Btmp,1)-2*padj(1),size(Btmp,2)-2*padj(2)];
score = mncc(awin,bwin,'mexfancc');

standoff = parms.template < 0;
hypos_clip = generate_hypotheses (score,standoff);

%% Col 1:2 = (r,c) absolute in image
%% Col 3 = score
%% Col 4:5 = (r,c) relative to window
%% Col 6 = theta (always zero here)
final_hypos = ...
    horzcat(hypos_clip(:,2:3) + ...
	    ones(size(hypos_clip,1),1)*rowcol - ss - 1, ...
	    hypos_clip, ...
	    zeros(size(hypos_clip,1),1));
