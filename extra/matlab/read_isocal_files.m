function isocal = read_isocal_files(tracking_fn, trackpoints_fn, adc_fn)

% tracking_fn = 'tracking_1.log';
% trackpoints_fn = 'trackpoints_1.log';
% adc_fn = 'angle-dependent-corrections_1.log';

[isocal.tracking.gantry,...
 isocal.tracking.dofsval,...
 isocal.tracking.x,...
 isocal.tracking.y,...
 isocal.tracking.z,...
 isocal.tracking.yaw,...
 isocal.tracking.pitch,...
 isocal.tracking.roll,...
 isocal.tracking.rmserr] = ...
    textread(tracking_fn,...
	     '%*d,"%*[^"]",%f,%d,%f,%f,%f,%f,%f,%f,%f',...
	     'headerlines',1);

[num_points,points] = ...
    textread(trackpoints_fn,...
	     '%*d,"%*[^"]",%*f,%d,%q',...
	     'headerlines',4);

%% Reshape points (string) into point_arr (array)
num_imgs = size(num_points,1);
for i=1:num_imgs
  s = points{i};
  a = sscanf(s,'%f,',17*3);
  point_arr{i}.points = reshape(a,3,num_points(i))';
end
isocal.trackpoints.point_arr = point_arr;

[isocal.adc.nominal_angle,...
 isocal.adc.found_angle,...
 isocal.adc.dx,...
 isocal.adc.dy,...
 isocal.adc.sid,...
 isocal.adc.sad,...
 filename] = ...
    textread(adc_fn,...
	     '%f,%f,%f,%f,%f,%f,%q',...
	     'headerlines',14);

%% Reindex adc file into filename order, such that 
%% file s is associated record isocal.adc.idx(s+1)
correction_valid = zeros(num_imgs,1);
correction_idx = zeros(num_imgs,1);
for i=1:size(isocal.adc.found_angle,1)
  %% Assumes filename is ...\x_???.viv, or ...\x_????.viv
  s = filename{i};
  s = s(max(strfind(s,'\'))+1:end-4);
  s = str2num(s(3:end));
  s = s + 1;  %% Images start at 1, filenames start at 0
  correction_valid(s) = 1;
  correction_idx(s) = i;
end
isocal.adc.idx = correction_idx;
isocal.adc.idx_valid = correction_valid;