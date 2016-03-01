clear -g curves;
clear -g depths;
clear -g energies;
clear -g sobp_range;
global curves;
global depths;
global energies;
global sobp_range;

%energies = 100:5:120;
energies = 100:3:120;
%energies = 100:120;
sobp_range = [76,102];

%energies = 95:114;
%sobp_range = [70,95];

%energies = 86:114;
%sobp_range = [60,95];

%energies = 81:120;
%sobp_range = [50,103]


for i=1:length(energies)
    fn = sprintf ('peak_%03d.txt', energies(i));
    d = load(fn);
    curves (:,i) = d(:,2);
end
depths = d(:,1);

% Optimize
initial_weights = 0.05 * ones (size(curves,2), 1);
weights = fminsearch ('search_metric', initial_weights);
weights = fminsearch ('search_metric', weights);
weights = fminsearch ('search_metric', weights);
weights = fminsearch ('search_metric', weights);
weights = fminsearch ('search_metric', weights);

search_metric(weights)

% Plot results
close all; figure; clf; hold on;
sobp = curves * weights;
plot(depths, sobp);
for i=1:length(energies)
    plot (depths, weights(i)*curves(:,i),'g');
end

% Make [PEAK] section
make_peaks = 0;
%make_peaks = 1;
if (make_peaks)
for i=1:length(energies)
    disp(sprintf('[PEAK]\nenergy=%f\nspread=%f\nweight=%f\n',...
                 energies(i),50,weights(i)));
end
end

