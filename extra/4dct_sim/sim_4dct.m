% function [object_recon, cps, center_of_mass, csav, volume, timesav, ...
%     z_c_of_m, order, num_shifts, lower_bound, upper_bound, cp_phase, rpm,
%     out] = ...
%     sim_4dct(opts)

function [out] = sim_4dct(opts)

% Uses an arbitrary object as the tumor for the simulation. Arbitrary 
% object is a 3D matrix.
% 
% function [ITVlim] = sim_4dct(run_name, filepath, slab_thickness, 
% slice_per_cm, pixel_size, scaling, phase_orig, beam_st_idx,time_shift, 
% hyst_delay, offset, ball_radius, xsize, ysize, row_for_im)
%
% This function creates an image of the slice generation for a particular 
% phase or phases.  It is has additional inputs, namely, beam_st_idx, which
% is the index of the RPM curve after which the scanner starts actually 
% slicing the sphere. It determines the slice indices using the phase info 
% from the .vxp file.  Therefore, phase_orig matters.
%
% Simulates hysteresis of internal and external motion. Hysteresis is
% simulated by a simple "delay" (0 to 250 ms) of the rpm trace. 
%
% This function also calculates the volume of the reconstructed image. It
% does this by counting "voxels."
%
% Mainly used to add some artificial baseline drift to a given RPM trace.
%
% ------------
% INPUTS:
% 
% object is a 3D matrix representing the arbitrary object to be
% scanned. Think of object as a stack of axial slices. Orientation is as
% follows: Given [x y z] == size(object), x and y specify the row and
% column of each axial image. z specifies the cranial-caudal direction
% (i.e. the axial image number. z = 1 corresponds to the axial slice at
% the most cranial part of the patient.) Assumes min value of all voxels
% is 0.
%
% Note about coordinates: The center of the object is considered to be:
% 
% [xsize ysize zsize] == size(object); % These are inputs from the user.
% xc = round(xsize/2); yc = round(ysize/2); zc = round(zsize/2);
% center_voxel = object(xc, yc, zc);
% 
% run_name is just a string that displays on the title of the figure. e.g.
% '2563'
% 
% filepath is the path to the filename of the .vxp file, without the .vxp
% extension. For example, filepath of c:/folder/person1.vxp is just
% 'c:/folder/person1'.  You can also use '..' and '.'.  If the .vxp file is
% in the current directory, the full path isn't required.
%
% slab_thickness is the thickness of each slice in cm.
% 
% slice_per_cm is the number of voxels per cm (along some axis). Used as a
% conversion factor to determine how much of the object is sliced given the
% desired slice thickness, etc.
%
% pixel_size is the length of a pixel in cm.
%
% scaling is the scale factor to convert the breathing trace specified by
% the .vxp file into tumor motion.
%
% phase_orig is the phase at which you want to construct the slice
% generation process.  It is given in percentages, such as 10, 20, 30, etc.
%
% beam_st_idx is the index of the RPM curve after which the scanner starts
% actually slicing the sphere.  If beam_st_idx indicates a point within a
% couch position, that couch position will be included as the first couch
% position where the sphere is scanned.  If beam_st_idx indicates a point
% outside a couch position, the first couch position after beam_st_idx will
% be the first couch position.
%
% time_shift is a variable that controls when the scanner begins slicing.
% time_shift is in seconds, and can be positive or negative.  Set it to 0
% for the original start time specified by the .vxp file.  NOTE: The time
% shift occurs after truncating the couch positions using beam_st_idx (see
% below).
%
% hyst_delay is the delay (in seconds) of the RPM trace (external motion).
% Typical values range from 0 to 250 ms, as found from Greg's fluoro
% studies. The internal motion of the marker leads the external motion of
% the RPM trace.
%
% offset is how far away from the object to begin slicing.
% 
% ball_radius is the radius of the ball in cm.
% 
% row_for_im is the row of each axial slice we are going to use to display
% the coronal cut in the figure.
% 
% ------------
% OUTPUTS:
%
% This function moves the wireframe sphere according to the .vxp breathing
% trace, and slices the sphere at the specified times.
%
% This function generates slice-time .jpg files in the following format:
% sim4d.run_name.phaseN.jpg
% It also can generate slice-time .jpg files of a contour-rendered volume
% in the following format:
% sim4d_Volume.run_name.phaseN.jpg
% N is the phase specified by phase_orig/10.
% 
% volume is the volume of the reconstructed image in units of centimeters 
% cubed. An xsize by ysize logical matrix is used for each CT slice, where 
% 1 corresponds to location where image exists. These matrices are stacked
% together to yield volume.
% 
% object_recon is the 4D-reconstructed object with the same dimensions and
% orientation of the input object matrix.
% 
% center_of_mass is the center of mass of the center of mass in the x, y,
% and z directions of object in pixels, given a particular phase. If the
% program is run over several phases (by setting collect_c_of_m = 1), csav
% will produce a list of each phase with its corresponding x, y, and z
% center of masses.
%
% Originally created by Alan Chu
%
% Modified by Joyatee Sarker
% 01/22/2008
%

% working on the presentation, and running patient data, test with the sine wave
% so we can understand the amplitude scaling

display_im = opts.display_im;
display_vol = opts.display_vol;
collect_c_of_m = opts.collect_c_of_m;
collect_time_offset = opts.collect_time_offset;
display_trace = opts.display_trace;
slice_per_cm = opts.slice_per_cm;
slab_thickness = opts.slab_thickness;
frames_per_sec = opts.frames_per_sec;
run_name = opts.run_name;
filepath = opts.filepath;
scaling = opts.scaling;
phase_orig = opts.phase_orig;
beam_st_idx = opts.beam_st_idx;
hyst_delay = opts.hyst_delay;
ball_radius = opts.ball_radius;
pixel_size = opts.pixel_size;
xsize = opts.xsize;
ysize = opts.ysize;
row_for_im = opts.row_for_im;
artificial_cp = opts.artificial_cp;
beam_on = opts.beam_on;
beam_off = opts.beam_off;
num_cps = opts.num_cps;
acq_time = opts.acq_time;
acqs_per_cp = opts.acqs_per_cp;
slice_thickness = slab_thickness / slice_per_cm;

%% To choose the display option %%
if display_im == 1
    h1 = figure;
end

if display_vol == 1
    h2 = figure;
end

if collect_c_of_m == 1
    phase_num = 0:10:90;
%     aviobj_cofm = avifile('center_of_mass.avi','FPS',1);
else phase_num = [phase_orig];
end

% if collect_c_of_m == 1
%     h3 = figure;
% end

if display_trace == 1
    h4 = figure;
    h5 = figure;
end

%% Find Filename (Prefix) out of Filepath %%
out.rpm = readrpm(filepath);      % Greg's RPM reader
% rpm.amp = -1*scaling*(rpm.amp - mean(rpm.amp)); % Make amplitudes relative to the mean amplitude
out.rpm.amp = out.rpm.amp - mean(out.rpm.amp); % Make amplitudes relative to the mean amplitude
out.rpm.amp = -1 * scaling * out.rpm.amp / std(out.rpm.amp) / (2*sqrt(2));
out.rpm.time = out.rpm.time - out.rpm.time(1); % Sets time to begin at 0.

out.offset = -2*std(out.rpm.amp) - ball_radius;     % Z-axis offset in order to not read useless (blank) images
offset = out.offset;

%% Adding BL drift %%
%slope = 9*0.0044 + 4e-4; % adjust for amount of drift
slope = 0; % no added drift
drift_line = slope*out.rpm.time;
out.rpm.amp_orig = out.rpm.amp;
out.rpm.amp = out.rpm.amp_orig + drift_line;

%% Hyst delay %%
hyst_delay_idx = extrap_time_to_index(out.rpm, hyst_delay); % number of indices to shift the rpm trace, as determined by hyst_delay
out.rpm.amp_internal = out.rpm.amp(hyst_delay_idx:end); % shifted rpm trace representing true internal motion
if hyst_delay_idx ~= 1
    out.rpm.amp_internal((end + 1):(end + hyst_delay_idx - 1)) = zeros(hyst_delay_idx - 1, 1);
end


if opts.mark_on == 1
    %% Setting phases based on marks %%
    out.cp_phase = out.rpm.mark;
    marks = find(out.cp_phase);
    speed = pi/mean(diff(marks));
    mark_values = out.cp_phase(marks);
    
    % Begin to 1st mark
    if mark_values(1)==1
      out.cp_phase(1:marks(1)) = linspace(max(pi,2*pi-speed*marks(1)), 2*pi,marks(1));
    else
      out.cp_phase(1:marks(1)) = linspace(max(0,pi-speed*marks(1)),pi,marks(1));
    end
    % Mark to mark
    for i=1:(size(marks)-1)
        if mark_values(i)==1
            out.cp_phase(marks(i):marks(i+1)) = linspace(0,pi,(marks(i+1)-marks(i)+1));
        else 
	  out.cp_phase(marks(i):marks(i+1)) = linspace(pi, 2*pi,(marks(i+1)-marks(i)+1));
        end
    end% Last mark to end
    trailing_len = size(out.cp_phase,1)-marks(end)+1;
    if mark_values(end)==1
        out.cp_phase(marks(end):end) = linspace(0,min(pi,speed*trailing_len),trailing_len);
    else 
      out.cp_phase(marks(end):end) = linspace(pi, min(2*pi,pi+speed*trailing_len),trailing_len);
    end
    %%
else
  out.cp_phase = out.rpm.phase;
end

if collect_time_offset == 1
    beam_st_idx = 1:100:(size(out.rpm.beam) - (beam_on + beam_off)*frames_per_sec*num_cps);
end

if (display_trace)
  figure(h4);clf;hold on;
  plot (out.rpm.time,out.rpm.amp_internal,'b');
  plot (out.rpm.time,out.rpm.phase - 10,'b');
  figure(h5);clf;hold on;
  plot ([out.rpm.time(1),out.rpm.time(end)],-[ball_radius,ball_radius],'c');
  plot ([out.rpm.time(1),out.rpm.time(end)],[ball_radius,ball_radius],'c');
end
    
out.object_recon = zeros(xsize, ysize, num_cps);

for time_offset_idx = 1:length(beam_st_idx);
    time_offset = beam_st_idx(time_offset_idx);

    if artificial_cp == 0
        %% Setting Couch Positions %%
        if ~isequal(out.rpm.beam, zeros(length(out.rpm.amp), 1))
            d = diff(out.rpm.beam);
            out.cps(:,1) = find(d==1) + 1; % Start of one couch position
            out.cps(:,2) = find(d==-1); % End of one couch position
        else error('Unable to determine beam on and off positions.');
        end

        %% Getting Rid of Couch Positions Before beam_st_idx %%
        for k = 1:size(out.cps, 1)
            if beam_st_idx < out.cps(k, 2)
                cps_st_idx = k;
                break;
            end
        end
        out.cps = out.cps(cps_st_idx:end, :);
        
    else
        out.cps(1,1) = time_offset;
        out.cps(1,2) = time_offset + beam_on*frames_per_sec - 1;
        for cp_idx = 2:num_cps
            out.cps(cp_idx,1) = out.cps((cp_idx - 1),2) + beam_off*frames_per_sec - 1;
            out.cps(cp_idx,2) = out.cps((cp_idx - 1),2) + beam_off*frames_per_sec + beam_on*frames_per_sec - 1;
        end
        out.cps = round(out.cps);
        if out.cps(end,2) > size(out.rpm.time)
            error('Too many couch positions, and not enough time.');
        end
    end

    for phase_index = 1:length(phase_num);
        phase_orig = phase_num(phase_index);
        phase = pi*phase_orig/50; % converting to radians
        
        out.volume = 0;
        sum_rad_square = 0;
        sum_cen_grav = 0;

	out.asav = [];
        out.dsav = [];
	out.z_idx_sav = [];
        for j = 1:num_cps
            start_idx = out.cps(j,1);                   % start index of jth cp
            end_idx = out.cps(j,2);                     % end index of jth cp
            
            cp_time = out.rpm.time(start_idx:end_idx);  % For the jth couch position, times of the indices in the cp
            cp_amp = out.rpm.amp_internal(start_idx:end_idx); % For the jth couch position, amplitudes of the indices in the cp
            acq_choices = round(linspace(extrap_time_to_index(out.rpm, acq_time/2 + out.rpm.time(start_idx)), ...
                extrap_time_to_index(out.rpm, out.rpm.time(end_idx) - acq_time/2), acqs_per_cp));
            
            poss_phases = out.cp_phase(acq_choices);
            
            [ph,ph_idx] = sort(abs(poss_phases-phase)); % Center phases around the wanted phase for the bin, and then sort so that the wanted phase is first
            cp_idx = acq_choices(ph_idx(1)) - start_idx + 1;  % cp index for the closest phase to the desired phase
            
            resp_translation = cp_amp(cp_idx);      % center = amp of the wanted bin
            couch_translation = j - 1;              % assumes 1 cm couch increments
            ball_translation = offset + resp_translation + couch_translation;
            
            d = linspace(slab_thickness,0,(slab_thickness*slice_per_cm + 1));
            d = d(2:end);                           % for each cp, 4 slices are observed; each cp = 1 cm
	    out.dsav = [out.dsav, d];
            a = (- ball_translation + d)/ball_radius; % convolution from CT coordinates to ball_translation (bt) coordinates require negative bt.
	    out.asav = [out.asav, a];
            r = (sin (acos (a)))*ball_radius;       % radius of the slice, in pixels
            r = r.*(a > -1);                        % lower threshold
            r = r.*(a < 1);                         % upper threshold
            rsav(j,:) = r;
            
            for e = 1:length(d)
                z_idx = j*length(d) - (length(d)-e);
		out.z_idx_sav = [out.z_idx_sav, z_idx];
                out.object_recon (:,:,z_idx) = 255 * makecircle(r(e), size(out.object_recon,2), size(out.object_recon,1), pixel_size);
                out.volume = out.volume + (1/(slice_per_cm))*pi*((r(e))^2);
                
                distance = z_idx;
                sum_rad_square = sum_rad_square + (r(e)^2);
                sum_cen_grav = sum_cen_grav + distance*(r(e)^2);
                
            end

            if (display_trace)
                figure(h4);
                plot (out.rpm.time(start_idx:end_idx),out.rpm.amp_internal(start_idx:end_idx),'r');
                plot (out.rpm.time(start_idx+cp_idx-1),resp_translation,'g^');
                plot (out.rpm.time(start_idx+cp_idx-1),out.rpm.phase(start_idx+cp_idx-1)-10,'k^');
                plot (out.rpm.time(acq_choices), out.rpm.amp_internal(acq_choices), 'mo');
                figure(h5);
                couch_wave = cp_amp + offset + couch_translation;
                plot (out.rpm.time(start_idx:end_idx),couch_wave,'m');
                plot (out.rpm.time(start_idx+cp_idx-1),couch_wave(cp_idx),'m^');
            end
        end % end of j
        
        if display_im == 1
            figure(h1);
            dsp_mod(reshape(out.object_recon(row_for_im, :, :), ysize, size(out.object_recon,3))');
            axis square;
            title([run_name ', Phase ' num2str(phase_orig) '%'], 'FontSize', 14, 'FontWeight', 'bold');
            saveas(h1, ['sim4d_' run_name '_phase' num2str(phase_orig/10) '.jpg']);
        end
        if display_vol == 1
            figure(h2);
            surface = smooth3(out.object_recon);
            vol_rend = contourslice(surface, [],[],[10:50],8);
            view(3);
            axis tight;
            set(vol_rend,'LineWidth',(36/5)); % 36/5 = points / 0.25 cm 
            lightangle(45,-45);
            title([run_name ', Phase ' num2str(phase_orig) '%'], 'FontSize', 14, 'FontWeight', 'bold');
            saveas(h2, ['sim4d_Volume_' run_name '_phase' num2str(phase_orig/10) '.jpg']);
        end

        out.center_of_mass = [(xsize + 1)/2,(ysize + 1)/2, ...
		    (sum_cen_grav/ sum_rad_square)] * ...
	    slice_thickness;  %% Center of mass in cm
        out.center_of_mass = out.center_of_mass + offset - slab_thickness; % Center of mass with ball centered at zero.
        out.csav(phase_index,:) = [time_offset, phase_orig, out.center_of_mass, out.volume];
    end   % end of phase_orig
    
%     if collect_c_of_m == 1
%         figure(h3); hold on;
%         plot(out.csav(:, 2), out.csav(:, 5), 'c.-');
%         title('Center of Mass of 10 phases', 'FontSize', 14);
%         xlabel('Phases (%)', 'FontSize', 12);
%         ylabel('Center of Mass (cm)', 'FontSize', 12);
% %         aviobj_cofm = addframe(aviobj_cofm,gca);
%     end

    out.timesav(:, :, time_offset_idx) = [out.csav];
    
end


if ndims(out.timesav) == 3 && collect_time_offset == 1
    out.z_c_of_m = out.timesav(:, 5, :);
    out.order = sort(out.z_c_of_m, 3);
    out.num_shifts = size(out.z_c_of_m,3);
    out.lower_bound = out.order(:,:,round(0.25*(out.num_shifts-1)+1));
    out.lower_bound = median(out.z_c_of_m, 3) - out.lower_bound;
    out.upper_bound = out.order(:,:,round(0.75*(out.num_shifts-1)+1));
    out.upper_bound = out.upper_bound - median(out.z_c_of_m, 3);
%     errorbar (phase_num, median(out.z_c_of_m, 3), out.lower_bound, out.upper_bound, 'b.-');
%     hold on;
%     plot(phase_num, out.order(:,:,end), 'm*');
%     plot(phase_num, out.order(:,:,1), 'm*');
%     saveas(h3, ['sim4d_CenterofMass_' run_name '.jpg']);
else
    out.z_c_of_m = 0;
    out.order = 0;
    out.num_shifts = 0;
    out.lower_bound = 0;
    out.upper_bound = 0;
end

% aviobj_cofm = addframe(aviobj_cofm,gca);
% aviobj_cofm = close(aviobj_cofm);

out.vol_calc = out.timesav(:, 6, :);
out.avgvol_per_phase = mean(out.vol_calc, 3);
out.vol_avg = mean(out.avgvol_per_phase);
out.vol_std = std(out.avgvol_per_phase);

out.avg_val = mean(out.z_c_of_m,3);
out.cofm_avg = mean(out.avg_val);
out.cofm_std = std(out.avg_val);