function hypo_list = generate_hypotheses(score,standoff,max_hypos)

%% max_hypos = 4;
srows = fix(size(standoff,1) / 2);
scols = fix(size(standoff,2) / 2);

for i=1:max_hypos
  [hypo_val,hypo_i] = max(score(:));
  [hypo_r,hypo_c] = ind2sub(size(score),hypo_i(1));
  hypo_list(i,:) = [hypo_val, hypo_r, hypo_c];
  stand_win = [hypo_r-srows,hypo_r+srows,hypo_c-scols,hypo_c+scols];
  score_win = clamp(stand_win, size(score));
  score(score_win(1):score_win(2),score_win(3):score_win(4)) = -1;
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Below is the old covariance estimation method, needs 
%% some work before it can be re-implemented
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if (0)
% [C_test,mx_test,my_test,C_mag_test] = est_cov(score,[21,21],0);

%% Alternate to est_cov method
[y,m] =  max(score(:));
[my_max,mx_max] = ind2sub(size(score),m);
%%    [my_max-my_test,mx_max-mx_test]

my_test = my_max;
mx_test = mx_max;
    
  L_auto = chol(C_auto);
  L_test = chol(C_test);
  L_diff = (L_test - L_auto);
  L = L_diff' * L_diff;

  if (C_mag_auto > C_mag_test)
    mag_diff = C_mag_auto/C_mag_test;
  else
    mag_diff = C_mag_test/C_mag_auto;
  end
  L = eye(2) + mag_diff * L;   %% eye(2) is sigma_min

  scs(si,thi).mag = C_mag_test;
  scs(si,thi).mag_a = C_mag_auto;
  scs(si,thi).C = C_test;
  scs(si,thi).C_a = C_auto;
  scs(si,thi).L = L;
  scs(si,thi).DL = det(L);
  scs(si,thi).score = score;
    
  scs(si,thi).thidx = thii;
  scs(si,thi).xy = [mx_test,my_test];
  scs(si,thi).maxs = max(max(score));
  scs(si,thi).score = score;

  scorestack(:,:,thi) = score;

  scsM = cat(1,scs(si,:).maxs);
  [mn,mni]=max(scsM);
  bestscore = mn;
  mx_test = scs(si,mni).xy(1);
  my_test = scs(si,mni).xy(2);
  th_test = parms.template_library.th_list(scs(si,mni).thidx);
end
