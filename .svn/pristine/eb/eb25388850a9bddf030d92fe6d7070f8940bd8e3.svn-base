function varargout = shackplot(varargin)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% shackplot.m - The quick, easy, versitile, and publication ready
%               "make you jump for joy and take the rest of the day off"
%               matlab plotting script for CSV formatted data
%
%     Depends - rotateXLabels.m
%               exportfig.m
%
%  EXAMPLE USAGE:
%
%  shackplot( ...
%      'csv/bspline_timing.csv',     ...
%      'eps',            'eps/bspline_timing.eps', ...
%      'epsmode',        'cmyk',    ...
%      'voledge',        'true',    ...
%      'legendline',     5,         ...
%      'csvstart',       6,         ...
%      'dataitems',      3,         ...
%      'style',          '---',     ...
%      'color',          'mkr',     ...
%      'xlabelsrotate',  'true',    ...
%      'xlabelskip',     20,        ...
%      'customaxis',     [10*10*10 500*500*500 0 700], ...
%      'xtitle',         'Volume Size (voxels)', ...
%      'ytitle',         'Execution Time (seconds)', ...
%      'xomit','7 11 15 19 23 27 31 35 39 43 51' ...
%  );
%
%
% Author: James Shackleford (tshack@drexel.edu)
%   Date: November 23rd, 2011
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Command line processing
%
%  shackplot(csv_in, [options,] ...)
%
%%%%%%%%%

if (nargin < 1)
    fprintf('Usage: shackplot(csv_in, [options,] ...)\n\n');
    fprintf(' OPTION       PARAMETER   DESCRIPTION\n');
    fprintf('   eps          (string ) name and path of eps output file\n');
    fprintf('   epsmode      (string ) options are "gray" "rgb" or "cmyk"\n');
    fprintf('   csvstart     (integer) line to start reading csv file from\n');
    fprintf('   legendline   (integer) line in csv file with column headers for legend\n');
    fprintf('   dataitems    (integer) # of traces in graph\n');
    fprintf('   style        (string ) style mask (one char per trace in graph)\n');
    fprintf('   color        (string ) color mask (one char per trace in graph)\n');
    fprintf('   xtitle       (string ) title of the x-axis\n');
    fprintf('   ytitle       (string ) title of the y-axis\n');
    fprintf('   xlabelrotate (string ) slant the x-axis labels (makes more space)\n');
    fprintf('   xlabelskip   (integer) x-axis label subsampling factor\n');
    fprintf('   customaxis   (vector ) 4x1 format [x-min x-max y-min y-max]\n');
    fprintf('   logx         (bool   ) use log x-axis\n');
    fprintf('   logy         (bool   ) use log y-axis\n');
    fprintf('   voledge      (bool   ) is the x-axis data a 3D volume edge length?\n');
    fprintf('   yprecision   (bool   ) 6-decimal precision instead of sci-notation\n');
    fprintf('   xomit        (integer) *indices* of x-axis tick labels to hide\n\n');
    return
end

% required parameters
csv_in = varargin{1};
if ~ischar(csv_in)
    error('First argument must be a string containing path to csv data file');
end

%eps_out = varargin{2};
%if ~ischar(eps_out)
%    error('Second argument must be a string containing the full desired output image path');
%end

% deal with optional parameters
paramPairs = {varargin{2:end}};
if nargin > 1
    if isstruct(paramPairs{1})
        pcell = LocalToCell(paramPairs{1});
        paramPairs = {pcell{:}, paramPairs{2:end}};
    end
end

if (rem(length(paramPairs),2) ~= 0)
    error(['Invalid input syntax. Optional parameters and values' ...
           ' must be in pairs.']);
end

% default option values
auto.eps_out = '';              % name and path of eps output file
auto.eps_mode = 'gray';         % options are 'gray' 'rgb' or 'cmyk'
auto.csvstart = 0;              % line to start reading csv file from
auto.legendline = 0;            % line in csv with column headers
auto.dataitems = 1;             % # of traces in graph
auto.style_cnt = 1;             % # of items in style mask (updated with mask)
auto.color_cnt = 1;             % # of items in color mask (updated with mask)
auto.style = '-';               % style mask (one line  code char per trace in graph)
auto.color = 'k';               % color mask (one color code char per trace in graph)
auto.xlabelsrotate = 'false';   % rotate the x-axis labels a bit? (to make more room)
auto.xlabelskip = 1;            % x-axis label subsampling factor
auto.customaxis = [];           % define custom x- and y-axis ranges
auto.logx = 'false';            % log x-axis?
auto.logy = 'false';            % log y-axis?
auto.voledge = 'false';         % is the x-axis quantity a 3D volume edge?
auto.xomit = '';                % x-axis tick *indices* to no label
auto.xomit_cnt = 0;             % # of xomit items (auto populated)
auto.yprecision = 'false';      % 6-decimal precision instead of sci-notation
auto.xtitle = 'x-axis';         % x-axis title
auto.ytitle = 'y-axis';         % y-axis title

for k = 1:2:length(paramPairs)
    param = lower(paramPairs{k});
    if ~ischar(param)
        error('Optional parameter names must be strings');
    end
    value = paramPairs{k+1};

    switch (param)
    case 'eps'
        if ~ischar(value) | strcmp(value,'auto')
            error('eps must be a string containing the full desired output image path');
        end
        auto.eps_out = value;
    case 'epsmode'
        if ~ischar(value) | ~(strcmp(value,'gray') | ~strcmp(value,'rgb') | ~strcmp(value,'cmyk'))
            error('epsmode must be a string: "gray"  "rgb"  "cmyk"');
        end
        auto.eps_mode = value;
    case 'csvstart'
        if ~(isnumeric(value) & (prod(size(value)) == 1) & (value >=1))
            error('csvstart must be >= 1');
        end
        auto.csvstart = LocalToNum(lower(value-1), auto.csvstart);
    case 'legendline'
        if ~(isnumeric(value) & (prod(size(value)) == 1) & (value >=1))
            error('legendline must be >= 1');
        end
        auto.legendline = LocalToNum(lower(value), auto.legendline);
    case 'dataitems'
        if ~(isnumeric(value) & (prod(size(value)) == 1) & (value >=1))
            error('dataitems must be >= 1');
        end
        auto.dataitems = LocalToNum(lower(value), auto.dataitems);
        % Grow style & color masks to match dataitems
        auto.style = '';    auto.style_cnt = auto.dataitems;
        auto.color = '';    auto.color_cnt = auto.dataitems;
        for i = 1:auto.dataitems
            auto.style = [auto.style '-'];
            auto.color = [auto.color 'k'];
        end
    case 'style'
        if ~ischar(value) | strcmp(value,'auto')
            error('style must be a string containing a valid style mask');
        end
        [auto.style, auto.style_cnt] = sscanf(value, '%c');
        if ~(auto.style_cnt == auto.dataitems)
            error('# of style mask elements must match dataitems');
        end
    case 'color'
        if ~ischar(value) | strcmp(value,'auto')
            error('color must be a string containing a valid color mask');
        end
        [auto.color, auto.color_cnt] = sscanf(value, '%c');
        if ~(auto.color_cnt == auto.dataitems)
            error('# of color mask elements must match dataitems');
        end
        if ~(auto.color_cnt == auto.style_cnt)
            error('# of color mask elements must match # of style mask elements');
        end
    case 'xtitle'
        if ~ischar(value)
            error('xtitle must be a character string');
        end
        auto.xtitle = value;
    case 'ytitle'
        if ~ischar(value)
            error('xtitle must be a character string');
        end
        auto.ytitle = value;
    case 'xlabelsrotate'
        if ~ischar(value)
            error('xlabelsrotate must be either "true" or "false"');
        end
        auto.xlabelsrotate = value;
    case 'xlabelskip'
        if ~(isnumeric(value) & (prod(size(value)) == 1) & (value >=1))
            error('csvstart must be >= 1');
        end
        auto.xlabelskip = LocalToNum(lower(value), auto.xlabelskip);
    case 'customaxis'
        auto.customaxis = value;
    case 'logx'
        if ~ischar(value)
            error('logx must be either "true" or "false"');
        end
        auto.logx = value;
    case 'logy'
        if ~ischar(value)
            error('logy must be either "true" or "false"');
        end
        auto.logy = value;
    case 'yprecision'
        if ~ischar(value)
            error('yprecision must be "true" or "false"');
        end
        auto.yprecision = value;
    case 'voledge'
        if ~ischar(value)
            error('voledge must be either "true" or "false"');
        end
        auto.voledge = value;
    case 'xomit'
        if ~ischar(value)
            error('xomit must contain a string of space separated x-axis tick indices to omit from labeling');
        end
        [auto.xomit, auto.xomit_cnt] = sscanf(value, '%i');
    otherwise
        error (['Unknown option ' param '.']);
    end
end


% read in csv file
csv_data = csvread(csv_in, auto.csvstart);

% maybe even make a legend
% (surely there's a better way... ianamp)
if ~auto.legendline == 0
    fid = fopen(csv_in, 'r');
    tline = fgetl(fid);
    for n = 2:auto.legendline
        tline = fgetl(fid);
    end
    fclose(fid);
    legend_cells = textscan(tline, '%s', 'delimiter',',');
    legend_labels = legend_cells{1};
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Construct the plot
%
%%%%%%%%%

figure(1);
hold;

% used to generate x-axis labels
x_range = csv_data(:,1);

% Compute @ of X axis ticks - y-axis datapoint per tick
%  this can change depending on input data
%  (if it is a cube edge for example, we need to cube it)
if strcmp(auto.voledge,'true')
    x_ticks = csv_data(:,1) .* csv_data(:,1) .* csv_data(:,1);
else
    x_ticks = csv_data(:,1);
end

% The total number of ticks (i.e. data points in a curve)
num_ticks = size(x_ticks,1);

% Build the x-axis labels
x_labels = cell(1,num_ticks);
if strcmp(auto.voledge,'true')
    for n = 1:num_ticks
        if mod(x_range(n,:), auto.xlabelskip) == 0
            x_labels(1,n) = {strcat(num2str(x_range(n,1)),'x',num2str(x_range(n,1)),'x',num2str(x_range(n,1)))};
        else 
            x_labels(1,n) = {''};
        end
    end
else
    for n = 1:num_ticks
        if mod(x_range(n,:), auto.xlabelskip) == 0
            x_labels(1,n) = {num2str(x_range(n,1))};
        else 
            x_labels(1,n) = {''};
        end
    end
end

% Selective x-axis tick label removal via 'xomit' option
for n = 1:auto.xomit_cnt
    x_labels(1,auto.xomit(n)) = {''};
end

% Plot each trace
%legend_labels = cell(1,auto.dataitems);
for n = 1:auto.dataitems
    trace = n+1;
    H = plot(x_ticks(1:num_ticks,1), csv_data(1:num_ticks,trace), strcat(auto.style(n),''));
    set(H, 'Color', auto.color(n));
    set(H, 'MarkerFaceColor', auto.color(n));
    set(H, 'MarkerSize', 6);
    set(H, 'LineWidth', 2);
end

% Mark up the axis with labels'n'setch
xlabel(auto.xtitle);
ylabel(auto.ytitle);
axis(auto.customaxis);
set(gca, 'XTick', x_ticks(1:num_ticks,1))
set(gca, 'XTickLabel', x_labels())
if strcmp(auto.xlabelsrotate, 'true')
    rotateXLabels(gca, 30);
end
if strcmp(auto.logx, 'true')
    set(gca, 'XScale', 'log')
end
if strcmp(auto.logy, 'true')
    set(gca, 'YScale', 'log')
end
if ~auto.legendline == 0
    legend(legend_labels(2:auto.dataitems+1), 'Location', 'NorthWest');
end
if strcmp(auto.yprecision,'true')
    old_ticks = get(gca, 'YTick')';
    new_tick_labels = cellfun(@(x) sprintf('%0.6f',x), num2cell(old_ticks), 'uniformoutput', false);
    set(gca, 'YTickLabel', new_tick_labels)
end
grid on;

if ~strcmp(auto.eps_out, '')
    fprintf ('Writing plot to disk: %s\n', auto.eps_out);
    exportfig(gcf, auto.eps_out, 'color', auto.eps_mode, 'width', 7, 'height', 4, 'fontmode','fixed', 'fontsize',8);
    close(1);
end


function value = LocalToNum(value,auto)
if ischar(value)
  if strcmp(value,'auto')
    value = auto;
  else
    value = str2num(value);
  end
end
