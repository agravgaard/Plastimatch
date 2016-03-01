function results = clip_track (parms)

% warning off MATLAB:conversionToLogical;

global WNCC_A WNCC_AW WNCC_B WNCC_BW;

verbose = 1;

num_clips = length(parms.tracks);

for t=1:num_clips
  switch (parms.template_type)
   case 'circle'
    tracks_cur(t).w = parms.ws;
    tracks_cur(t).len = parms.ws;
    tracks_cur(t).th = 0;
    tracks_cur(t).rowcol_cur = parms.tracks(t).start_parms(1:2);
    tracks_cur(t).rowcol_ori = parms.tracks(t).start_parms(1:2);
   case 'cylinder'
    tracks_cur(t).w = parms.tracks(t).start_parms(1);
    tracks_cur(t).len = parms.tracks(t).start_parms(2);
    tracks_cur(t).th = parms.tracks(t).start_parms(3);
    tracks_cur(t).rowcol_cur = parms.tracks(t).start_parms(4:5);
    tracks_cur(t).rowcol_ori = parms.tracks(t).start_parms(4:5);
  end
end

%% Window size of pattern
ws = parms.ws;

%% Search window size
ss = parms.ss;

%% RUN LOOP
frames = parms.first_frame:parms.last_frame;
for si=1:length(frames)

%  if (si == 1 || mod(si,10)==0)
    printf ('---- FRAME %d', si);
%  end

  fn = parms.dirlist{frames(si)};
  if (strcmp(fn(end-3:end),'.viv') || strcmp(fn(end-3:end),'.raw'))
    Bori = double(readviv(fn));
  else
    Bori = double(imread(fn));
  end

  %% The reason for having a B and a Bori is because my old code 
  %% for the ximatron fluoro did some intensity rescaling
  
  %% The panels sometimes have bad pixels, so we can do med filt
  B = medfilt2(Bori,[3,1]);
  B = medfilt2(B,[1,3]);

  BW = ones (size(B));

  %% NCC Spatial fit with old parmsa
  %% -- Btmp is the window in B
  %% -- BWtmp is the weight of the window in B
  clear ('hypos');
  for t=1:num_clips
    w = tracks_cur(t).w;
    len = tracks_cur(t).len;
    th = tracks_cur(t).th;
    rowcol = tracks_cur(t).rowcol_cur;
    rows = rowcol(1)-ws-ss:rowcol(1)+ws+ss;
    cols = rowcol(2)-ws-ss:rowcol(2)+ws+ss;
    if (rows(1) <= 0)
      rows = rows - rows(1) + 1;
    elseif (rows(end) > size(B,1))
      rows = rows + size(B,1) - rows(end);
    end
    if (cols(1) <= 0)
      cols = cols - cols(1) + 1;
    elseif (cols(end) > size(B,2))
      cols = cols + size(B,2) - cols(end);
    end
    Btmp = B(rows,cols);
    BWtmp = BW(rows,cols);

    %% Here is where I could robustify the intensities, but I don't 
    %% do that
    
    %% Match at a single l, and a set of thetas
    switch (parms.template_type)
     case 'circle'
      results.tracks(t).hypos{si} ...
	  = match_circle(parms,rowcol,Btmp,BWtmp);
     case 'cylinder'
      results.tracks(t).hypos{si} ...
	  = match_cylinder(parms,th,len,rowcol,Btmp,BWtmp);
    end
    hypos{t} = results.tracks(t).hypos{si};
  end

  %% Resolve hypothesis
  assignment_no = 0;
  best_assignment = [];
  best_assignment_prob = 0;
  rowcol_cur = reshape([tracks_cur(:).rowcol_cur],2,num_clips)';
  while (1)
    assignment = create_assignment(hypos,assignment_no);
    if (isempty(assignment))
      assignment = ones(num_clips,1);
      break;
    end
    assignment_prob = score_assignment (hypos, assignment, rowcol_cur);
    if (assignment_prob > best_assignment_prob)
      best_assignment_prob = assignment_prob;
      best_assignment = assignment;
    end
    assignment_no = assignment_no + 1;
  end
  if (isempty(best_assignment))
    %% format short g;
    %% for t=1:num_clips
    %%   hypos{t}
    %% end
    %% save;
    %% error ('Error finding assignment');
    
    %% crapping out -- use interactive positioning
    interactive_results = interactive (B,rowcol_cur);
    for t=1:num_clips
      hypos{t} = [interactive_results(t,:), 0, 0, 0, tracks_cur(t).th];
    end
    best_assignment = ones(1,num_clips);
  else
    score_assignment (hypos, best_assignment, rowcol_cur);
  end

  for t=1:num_clips
    %% Update parameters (if tracking accepted)
    measurement_accepted = 1;

    best_rowcol = hypos{t}(best_assignment(t),1:2);
    best_score = hypos{t}(best_assignment(t),3);
    best_dxy = hypos{t}(best_assignment(t),4:5);
    best_theta = hypos{t}(best_assignment(t),6);
    
    if (best_score < 0.5)
      measurement_accepted = 0;
    end

%     if (~measurement_accepted)
%       disp('Warning: extrapolating.');
%     end

    tracks_cur(t).rowcol_cur = best_rowcol;
    tracks_cur(t).th = best_theta;
    results.tracks(t).w(si) = tracks_cur(t).w;
    results.tracks(t).len(si) = tracks_cur(t).len;
    results.tracks(t).th(si) = best_theta;
    results.tracks(t).rowcol(si,:) = round(best_rowcol);
    results.tracks(t).best_score(si) = best_score;

    printf ('[C%d: [%#7.6g,%#7.6g,%g,%g]]', t, ...
	    best_rowcol(1), best_rowcol(2), best_theta, best_score);
  end
  
  %% Make imgs
  if (parms.display_rate > 0 && ...
      (mod(si-1,parms.display_rate)==0 || si==length(frames)))

    rcmin = min(vertcat(tracks_cur(:).rowcol_ori)) - [150,100];
    rcmax = max(vertcat(tracks_cur(:).rowcol_ori)) + [150,100];
    win = clamp([rcmin(1),rcmax(1),rcmin(2),rcmax(2)],size(Bori));
    xr = win(3):win(4);
    yr = win(1):win(2);

    %% Display image
    Bpat = Bori;
    Bpat(Bpat < 0) = 0;
    Bpat(Bpat > 1) = 1;
    aviimg_1 = my_histeq_2(Bori(yr,xr));
    Bpat(~BW) = 0.75;

    figure(1);clf;
    image(xr,yr,aviimg_1);colormap(gray(256));
    axis equal;
    axis tight;
    hold on;
    
    for t=1:num_clips
      len = results.tracks(t).len(si);
      w = results.tracks(t).w(si);
      th = results.tracks(t).th(si);
      rowcol = results.tracks(t).rowcol(si,:);
      ll = len + 9;
      ww = w + 7;
      lx1 = rowcol(2) - ll * cos(th);
      lx2 = rowcol(2) + ll * cos(th);
      ly1 = rowcol(1) + ll * sin(th);
      ly2 = rowcol(1) - ll * sin(th);
      lx3 = rowcol(2) - ll * sin(th);
      lx4 = rowcol(2) + ll * sin(th);
      ly3 = rowcol(1) - ll * cos(th);
      ly4 = rowcol(1) + ll * cos(th);

      bx1 = rowcol(2) - (ll+1) * cos(th) + (ww+1) * sin(th);
      bx2 = rowcol(2) - (ll+1) * cos(th) - (ww+1) * sin(th);
      bx3 = rowcol(2) + (ll+1) * cos(th) + (ww+1) * sin(th);
      bx4 = rowcol(2) + (ll+1) * cos(th) - (ww+1) * sin(th);
      by1 = rowcol(1) + (ll+1) * sin(th) + (ww+1) * cos(th);
      by2 = rowcol(1) + (ll+1) * sin(th) - (ww+1) * cos(th);
      by3 = rowcol(1) - (ll+1) * sin(th) + (ww+1) * cos(th);
      by4 = rowcol(1) - (ll+1) * sin(th) - (ww+1) * cos(th);

      plot([bx1;bx2;bx4;bx3;bx1],...
	   [by1;by2;by4;by3;by1],'r-');
      
    end
    drawnow;
    pause;
  end
  
%   if (si==10)
%     w = mean(flt.w(1:10));
%     l = mean(flt.l(1:10));
%     fullsearch = 0;
%   end
  
end

results.dirlist = parms.dirlist;
results.timestamp = parms.timestamp;
results.frame_no = parms.frame_no;
results.first_frame = parms.first_frame;
results.last_frame = parms.last_frame;

save(parms.out_file,'results');
close all;
