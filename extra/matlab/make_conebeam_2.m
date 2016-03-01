clear; close all;

%%%%%%%%%% These are for the calibration frames %%%%%%%%%%%%%%%%%%%%%%
%% Note: dropped of 151 implies the frame was dropped between 151 and 152
%% Frames have to be specified in terms of renamed idxs 
%% --- '2006-05-26'
% caldir='2005-11-12';
% isocal='a';
% imager='1';
% first_frame = 42;
% last_frame = 512;

%% --- '2006-05-26'
% caldir='2006-05-26';
% isocal='b';
% imager='2';
% dropped = [ 151, 461 ];
% first_frame_in = 7;
% last_frame_in = 473 + length(dropped);

%% --- '2008-03-13'
caldir='c:/gsharp/projects/isocal-interpret/2008-03-13/';
isocal='b';
imager='1';
dropped = [ ];
first_frame_in = 6;
last_frame_in = 471;

%% There need to be same number of output frames as input frames
nframes_in = last_frame_in - first_frame_in - length(dropped) - 1;

%%%%%%%%%% These are for the reconstruction image frames %%%%%%%%%%%%
%% Note: dropped of 151 implies the frame was dropped between 151 and 152
%% --- 0005a 
first_frame_out = 36;
last_frame_out = 503;
reverse_order = 0;
dropped_out = [ 103 ];
txt_outdir = 'c:/gsharp/idata/mghdrr/0005a_small_filt/';

%% --- 0005c
first_frame_out = 2603;
%% last_frame_out = 3063;
last_frame_out = 3071;
reverse_order = 1;
dropped_out = [];
txt_outdir = 'c:/gsharp/idata/mghdrr/0005c_small_filt/';

%% --- 0005c
first_frame_out = 2603;
%% last_frame_out = 3063;
last_frame_out = 3071;
reverse_order = 1;
dropped_out = [];
txt_outdir = 'c:/gsharp/idata/mghdrr/0005c_small_filt/';

%% --- 2006-12-11/0000b
%% first_frame_out = 549;
%% last_frame_out = 1011;
first_frame_out = 546;
last_frame_out = 1014;
reverse_order = 1;
dropped_out = [];
txt_outdir = 'C:/gsharp/idata/gpuit-data/2006-12-11/0000b-small-filt/';

%% --- 2006-12-11/0000b
%% first_frame_out = 549;
%% last_frame_out = 1011;
first_frame_out = 546;
last_frame_out = 1014;
reverse_order = 1;
dropped_out = [];
txt_outdir = 'C:/gsharp/idata/gpuit-data/2006-12-11/0000b-small-filt/';

%% --- 2007-02-08/0002
first_frame_out = 2010;
%last_frame_out = 2478;
last_frame_out = 2464;
reverse_order = 1;
dropped_out = [];
txt_outdir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0002-small-filt/';

%% --- 2006-09-12/0001a-final
first_frame_out = 307;
last_frame_out = 770;
truncate_end = 0;
reverse_order = 0;
dropped_out = [];
txt_outdir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a-final/';

%% --- 2008-03-13
%% Should be approx 3074 - 3540
first_frame_out = 3075;
last_frame_out = first_frame_out + nframes_in - 1;
truncate_end = 0;
reverse_order = 0;
dropped_out = [];
txt_outdir = 'g:/reality/2008-03-13/0000-final/';


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
load([caldir '/markers.mat']);

tracking_fn = [caldir '/isocal-' isocal '/tracking_' imager '.log'];
trackpoints_fn = [caldir '/isocal-' isocal '/trackpoints_' imager '.log'];
adc_fn = [caldir '/isocal-' isocal ...
	  '/angle-dependent-corrections_' imager '.log'];
isocal = read_isocal_files(tracking_fn, trackpoints_fn, adc_fn);

if (imager == '1')
  marker_locs_270 = m1_270;
%  marker_locs_315 = m1_315;
  marker_locs_0 = m1_0;
  marker_locs_90 = m1_90;
else  
  marker_locs_225 = m2_225;
%  marker_locs_270 = m2_270;
  marker_locs_315 = m2_315;
%  marker_locs_0 = m2_0;
  marker_locs_45 = m2_45;
%  marker_locs_90 = m2_90;
end

%% marker_locs = m2_225;
%% m_225 = fit_markers_to_trackpoints(isocal, marker_locs_225);
m_270 = fit_markers_to_trackpoints(isocal, marker_locs_270);
%% m_315 = fit_markers_to_trackpoints(isocal, marker_locs_315);
m_0 = fit_markers_to_trackpoints(isocal, marker_locs_0);
%% m_45 = fit_markers_to_trackpoints(isocal, marker_locs_45);
m_90 = fit_markers_to_trackpoints(isocal, marker_locs_90);

%% We need to add some extra stuff for the dropped frames in the 
%% isocal sequence

for i=1:length(dropped)
  idx1 = isocal.adc.idx(dropped(i));
  idx2 = isocal.adc.idx(dropped(i)+1);
  new_idx = length(isocal.adc.idx) + i;
  
  fa1 = isocal.adc.found_angle(idx1);
  fa2 = isocal.adc.found_angle(idx2);
  isocal.adc.found_angle(new_idx) = (fa1 + fa2) / 2;
  fprintf(1,'%g %g -> %g\n', fa1, fa2, isocal.adc.found_angle(new_idx));

  sa1 = isocal.adc.sad(idx1);
  sa2 = isocal.adc.sad(idx2);
  isocal.adc.sad(new_idx) = (sa1 + sa2) / 2;

  si1 = isocal.adc.sid(idx1);
  si2 = isocal.adc.sid(idx2);
  isocal.adc.sid(new_idx) = (si1 + si2) / 2;

  dx1 = isocal.adc.dx(idx1);
  dx2 = isocal.adc.dx(idx2);
  isocal.adc.dx(new_idx) = (dx1 + dx2) / 2;

  dy1 = isocal.adc.dy(idx1);
  dy2 = isocal.adc.dy(idx2);
  isocal.adc.dy(new_idx) = (dy1 + dy2) / 2;
end

for i=1:length(dropped)
  new_idx = length(isocal.adc.idx) + 1;
  isocal.adc.idx = [isocal.adc.idx(1:dropped(i)+i-1);...
		    new_idx;...
		    isocal.adc.idx(dropped(i)+i:end)];
end

fullrez = 0;
columns_are_flipped = 0;
tgt = [0;0;0];
vup = [0;0;1];
ps = 0.194 * 2;
if (fullrez)
  ps = ps / 2;
end

in_list  = first_frame_in:last_frame_in;
out_list = first_frame_out:last_frame_out;

if (length(out_list) < length(in_list))
  slop = length(in_list) - length(out_list);
  if (truncate_end)
    out_list = [out_list, -1 * ones(1, slop)];
  else
    out_list = [-1 * ones(1, slop), out_list];
  end
end


%% Remove dropped outputs from in_list
in_list_mask = ones(size(in_list));
if (~isempty(dropped_out))
  in_list_mask(dropped_out+[1:length(dropped_out)]) = 0;
end
in_list = in_list(logical(in_list_mask));

%% Flip if reverse order
if (reverse_order)
  out_list = fliplr(out_list);
end

for i=1:length(in_list)
  in_idx = in_list(i);
  out_idx = out_list(i);
  if (out_idx == -1)
    continue;
  end
  
  idx = isocal.adc.idx(in_idx);
  ang(i) = isocal.adc.found_angle(idx) * pi / 180;
  sad(i) = isocal.adc.sad(idx) * 10;                     %% cm -> mm
  sid(i) = isocal.adc.sid(idx) * 10;                     %% cm -> mm
  ic(i,:) = [isocal.adc.dy(idx), isocal.adc.dx(idx)];
  if (fullrez)
    ic(i,:) = ic(i,:) * 2;
  end
  
%  cam0(:,i) = [cos(ang(i));-sin(ang(i));0];
%  cam0(:,i) = [-sin(ang(i));cos(ang(i));0];
  cam0(:,i) = [sin(ang(i));-cos(ang(i));0];
  nrm(:,i) = tgt - cam0(:,i);
  nrm(:,i) = nrm(:,i) / norm(nrm(:,i));
  tmp = sad(i) * nrm(:,i);
  cam(:,i) = tgt - tmp;
  
  vrt(:,i) = cross(nrm(:,i), vup);
  vrt(:,i) = vrt(:,i) / norm(vrt(:,i));
  vup1(:,i) = cross (vrt(:,i), nrm(:,i));
  vup1(:,i) = vup1(:,i) / norm(vup1(:,i));
  
  nrm(:,i) = - nrm(:,i);
  
  extrinsic = zeros(4,4);
  extrinsic(1,1:3) = vrt(:,i)';
  extrinsic(2,1:3) = vup1(:,i)';
  extrinsic(3,1:3) = nrm(:,i)';
  extrinsic(3,4) = - sad(i);
  extrinsic(4,4) = 1;
  extr{i} = extrinsic;
  
  intrinsic = zeros(3,4);
  intrinsic(1,2) = - 1 / ps;
  if (columns_are_flipped)
    intrinsic(2,1) = - 1 / ps;
  else
    intrinsic(2,1) = 1 / ps;
  end
  intrinsic(3,3) = - 1 / sid(i);
  intr{i} = intrinsic;
  
  projection = intrinsic * extrinsic;

  fn = sprintf([txt_outdir 'out_%04d.txt'],out_idx);
  fp = fopen (fn,'w');
  fprintf (fp, '%18.8e %18.8e\n',ic(i,1),ic(i,2));
  for j=1:3
    fprintf (fp, '%18.8e ',projection(j,:));
    fprintf (fp, '\n');
  end
  fprintf (fp, '%18.8e\n', sad(i));
  fprintf (fp, '%18.8e\n', sid(i));
  fprintf (fp, '%18.8e ', nrm(:,i));
  fprintf (fp, '\n');
  fclose (fp);
end
