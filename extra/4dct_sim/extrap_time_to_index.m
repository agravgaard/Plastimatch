function index = extrap_time_to_index(rpm, time)
%   extrap_time_to_index(rpm, time) takes in a time and outputs the index
%   of the rpm structure that most closely corresponds to that time.  In
%   particular, it compares 'time' to the two closest values in the vector,
%   rpm.time, and the index of whichever value is closer to the desired
%   'time' is produced.

low_time_vector = find(rpm.time <= time);
low_time_index = low_time_vector(end);
if low_time_index == length(rpm.time) % return 0 if 'time' is out of bounds
    index = 0;
    return;
end
low_time = rpm.time(low_time_index);
high_time_index = low_time_index + 1;
high_time = rpm.time(high_time_index);
if (time - low_time) < (high_time - time)
    index = low_time_index;
else
    index = high_time_index;
end