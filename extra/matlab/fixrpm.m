function rpm_out = fixrpm(rpm)
% FIXRPM Fix Varian RPM data to interpolate missing timesteps
%    RPM_OUT = fixrpm (RPM)

%% This only works for interpolating single values
timediff = rpm.time(2:end)-rpm.time(1:end-1);
interp_idx = find (timediff > 1.5*median(timediff));

rpm_out.amp (1:interp_idx(1)) = rpm.amp (1:interp_idx(1));
rpm_out.phase (1:interp_idx(1)) = rpm.phase (1:interp_idx(1));
rpm_out.time (1:interp_idx(1)) = rpm.time (1:interp_idx(1));
rpm_out.valid (1:interp_idx(1)) = rpm.valid (1:interp_idx(1));
rpm_out.ttlin (1:interp_idx(1)) = rpm.ttlin (1:interp_idx(1));
rpm_out.mark (1:interp_idx(1)) = rpm.mark (1:interp_idx(1));
rpm_out.ttlout (1:interp_idx(1)) = rpm.ttlout (1:interp_idx(1));
for i=1:length(interp_idx)
    this_idx = interp_idx(i);
    if (i == length(interp_idx))
        next_idx = length(rpm.time);
    else
        next_idx = interp_idx(i+1);
    end
    
    %% Add interpolated value
    rpm_out.amp(this_idx+i) = ...
        (rpm.amp(this_idx) + rpm.amp(this_idx+1)) / 2;
    if (rpm.phase(this_idx+i) < rpm.phase(this_idx))
        %% Should do mod 2 pi interpolation here, but this may be enough
        rpm_out.phase(this_idx+i) = 0;
    else
        rpm_out.phase(this_idx+i) = ...
            (rpm.phase(this_idx) + rpm.phase(this_idx+1)) / 2;
    end
    rpm_out.time(this_idx+i) = ...
        (rpm.time(this_idx) + rpm.time(this_idx+1)) / 2;
    rpm_out.valid(this_idx+i) = 0;
    rpm_out.ttlin(this_idx+i) = rpm.ttlin(this_idx);
    rpm_out.mark(this_idx+i) = 0;
    rpm_out.ttlout(this_idx+i) = rpm.ttlout(this_idx);
    

    %% Copy over values that don't need interpolation
    rpm_out.amp(this_idx+i+1:next_idx+i) = rpm.amp(this_idx+1:next_idx);
    rpm_out.phase(this_idx+i+1:next_idx+i) = rpm.phase(this_idx+1:next_idx);
    rpm_out.time(this_idx+i+1:next_idx+i) = rpm.time(this_idx+1:next_idx);
    rpm_out.valid(this_idx+i+1:next_idx+i) = rpm.valid(this_idx+1:next_idx);
    rpm_out.ttlin(this_idx+i+1:next_idx+i) = rpm.ttlin(this_idx+1:next_idx);
    rpm_out.mark(this_idx+i+1:next_idx+i) = rpm.mark(this_idx+1:next_idx);
    rpm_out.ttlout(this_idx+i+1:next_idx+i) = rpm.ttlout(this_idx+1:next_idx);
end

%% Copy over other values
rpm_out.header = rpm.header;
rpm_out.version = rpm.version;
