function cps_final = shiftcps(cps, rpm, time_shift)
%   cps_final = shiftcps(cps, rpm, time_shift) shifts the couch positions
%   to begin at an arbitrary time shift, time_shift (in sec.).
%
%   rpm is the structure returned by Greg Sharp's readrpm() function, and
%   contains the information from the rpm file.
%
%   cps is a matrix of starting and ending points of all the couch
%   positions.  For example, cps(i, 1) is the index of the starting point
%   of the ith couch position, and cps(i, 2) is the index of the end point
%   of the ith couch position.  The size of cps is therefore i by
%   2.
%
%   cps_final is of the exact same form as cps.  cps_final contains the
%   starting and ending points of all the shifted couch positions.

if time_shift == 0                 % Use the original couch positions
    cps_final = cps;
    return;
else
    for k = 1:length(cps)
        % shift start and end points of couch positions
        cps_final(k, 1) = extrap_time_to_index(rpm, rpm.time(cps(k, 1)) + time_shift);
        cps_final(k, 2) = extrap_time_to_index(rpm, rpm.time(cps(k, 2)) + time_shift);
        if cps_final(k, 1) == 0 | cps_final(k, 2) == 0 % make sure the shift
            if k > 1                                   % doesn't move out of bounds
                cps_final = cps_final(1:k - 1, :);
                return;
            else
                %error('Time shift is too large.');
                cps_final = 0;
                return;
            end
        end
    end
end