function [beam_on, beam_off] = find_true_beam_on_off(filepath);

rpm = readrpm(filepath);      % Greg's RPM reader
d = diff(rpm.beam);
cps(:, 1) = find(d==1) + 1;
cps(:, 2) = find(d== -1) + 1;

beam_on = mean(rpm.time(cps(:,2)) - rpm.time(cps(:,1)));
cps_new = [cps(2:end, 1), cps(1:end-1, 2)];
beam_off = mean(rpm.time(cps_new(:,1)) - rpm.time(cps_new(:, 2)));
