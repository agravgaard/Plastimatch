% close all;
clear all;

opts.display_im = 1;             % display image option
opts.display_vol = 0;            % display volume option
opts.collect_c_of_m = 0;         % plot center of masses over a range of phases [0:10:90]
opts.collect_time_offset = 0;    
opts.display_trace = 1;          % plot rpm trace used
opts.mark_on = 0;                % calculate phase based on marks?

%% These match our hardware
opts.slice_per_cm = 4;           % slices per centimeter
opts.slab_thickness = 1.0;       % thickness of the slab in centimeters
opts.frames_per_sec = 30;        % # of images per second.

opts.run_name = 'Volume Rendering';    
% opts.filepath = './4dct-traces/sine.vxp';
% files = dir('./4dct-traces/*.vxp');
opts.filepath = './sine.vxp';

%% Scaling: [1, 2, 3, 4]
opts.scaling = 6;                % mag factor

%% Phase: [0, 10, ..., 90]
opts.phase_orig = 30;            % phase origin

%% Beam_st_idx: [1 .. max]
opts.beam_st_idx = 100;         % index where the beam begins capturing images
                            % like a time offset

%% Hyst_delay = 0
opts.hyst_delay = 0.250/8*0;

%% Radius: [2, 4, 6]
opts.ball_radius = 2;            % radius of the sphere in centimeters

%% Pixel_size = 1 mm
opts.pixel_size = .1;            % pixel size in cm

%% As appropriate
opts.xsize = 121;
opts.ysize = 121;
opts.row_for_im = floor(opts.xsize / 2);   % row of the x-y object to display in z-coordinates

%% yes
opts.artificial_cp = 1;          % to use (1) or not use (0) synthetically produced cps
opts.beam_on = 5;                % duration in seconds xray beam is on
opts.beam_off = 1.5;             % duration in seconds xray beam is off
% [opts.beam_on, opts.beam_off] = find_true_beam_on_off(opts.filepath);

%% As appropriate
opts.num_cps = 25;               % only matters if artifical_cp is set to 1
opts.acq_time = 0.8;        % Tube spin time (in seconds)
opts.acqs_per_cp = 12;      % Number of scanner acquisitions per couch
                            % pos.

[out] = sim_4dct(opts);
% 
% for f = 1:length(files)
%     opts.filepath = ['./4dct-traces/' files(f).name];
%     [opts.beam_on, opts.beam_off] = find_true_beam_on_off(opts.filepath);
%     [out] = sim_4dct(opts);
%     datasav(f,:) = [out.vol_avg, out.vol_std, out.cofm_avg, out.cofm_std]
% end

% for f = 1:length(files)
%     opts.filepath = ['./4dct-traces/' files(f).name];
%     [opts.beam_on, opts.beam_off] = find_true_beam_on_off(opts.filepath);
%     [out] = sim_4dct(opts);
%     datasav(f,:) = [mean(out.z_c_of_m), std(out.z_c_of_m), mean(out.vol_calc), std(out.vol_calc)]
% end