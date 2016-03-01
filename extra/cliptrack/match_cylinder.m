function final_hypos = match_cylinder (parms, th, len, rowcol, Btmp, BWtmp)

global WNCC_A WNCC_AW WNCC_B WNCC_BW;

%% How close should two hypotheses be before they are considered the
%% same (unit = pixels squared), for purposes of compressing 
%% across different angles
unique_hypo_sqdist_thresh = 16;

%% How many hypotheses per clip?
max_hypos = 4;
max_hypos = 5;

% alg = 'mexfncc';
% alg = 'mexfancc';
alg = 'mexmatch_fncc';
% alg = 'mexmatch_fancc';

ws = parms.ws;
ss = parms.ss;

th_step = parms.template_library.th_step;
th_list = parms.template_library.th_list;
thai = round(th*180/pi/th_step) + 1;
tha = ((thai - 1)*th_step) * pi/180;
%%  thtest_list = thai-2:thai+2;
thtest_list = thai-1:thai+1;
thtest_list(thtest_list < 1) = thtest_list(thtest_list < 1) ...
    + length(th_list);
thtest_list(thtest_list > length(th_list)) ...
    = thtest_list(thtest_list > length(th_list)) ...
    - length(th_list);
[ld,li] = min((parms.template_library.l_list - len).^2);

all_hypos = [];
for thi = 1:length(thtest_list)
  thii = thtest_list(thi);
  %% p1 is the pattern (for this length and theta)
  %% w1 is the pattern weight (for this length and theta)
  p1 = parms.template_library.template{li,thii,1};
  w1 = parms.template_library.template{li,thii,2};
  psz = parms.template_library.template{li,thii,3};
  WNCC_A = p1;
  WNCC_AW = w1;
  WNCC_B = Btmp;
  WNCC_BW = BWtmp;

  awin = [1,1,size(p1,1),size(p1,2)];
  padj = [ws,ws] - psz;
  bwin = [padj(1)+1,padj(2)+1,...
	  size(Btmp,1)-2*padj(1),size(Btmp,2)-2*padj(2)];
  score = mncc(awin,bwin,alg,0.1,7.0);

  standoff = p1 < .9;
  new_hypos = generate_hypotheses (score,standoff,max_hypos);
  all_hypos = [all_hypos;new_hypos,th_list(thii)*ones(size(new_hypos,1),1)];
end

%% Compress hypotheses for each clip
hypos_tmp = all_hypos;
hypos_tmp = sortrows(hypos_tmp,1);
hypos_tmp = flipud(hypos_tmp);
hypos_clip = hypos_tmp(1,:);
for i=2:size(hypos_tmp,1)
  d = find_sq_distances (hypos_tmp(i,2:3), hypos_clip(:,2:3));
  if (min(d) > unique_hypo_sqdist_thresh)
    hypos_clip = [hypos_clip; hypos_tmp(i,:)];
  end
  if (size(hypos_clip,1) >= max_hypos)
    break;
  end
end

%% Col 1:2 = (r,c) absolute in image
%% Col 3 = score
%% Col 4:5 = (r,c) relative to window
%% Col 6 = theta
final_hypos = ...
    horzcat(hypos_clip(:,2:3) + ...
	    ones(size(hypos_clip,1),1)*rowcol - ss - 1, ...
	    hypos_clip);
