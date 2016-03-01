function rpm = readrpm(fn)
% READRPM Read Varian RPM format file
%    RPM = READRPM(FN)
%
%    Read Varian RPM format, either DAT or VXP, and store the results
%    in the structure RPM, which has the following fields:
%
%    RPM.VERSION  String indicating the file type.  Valid values are 
%                   'DAT 1.4', 'DAT 1.7', 'VXP 1.5', or 'VXP 1.6'
%    RPM.AMP      Nx1 array with position in centimeters relative to 
%                   an arbitrary reference
%    RPM.PHASE    Nx1 array with phase value for this sample
%    RPM.TIME     Nx1 array indicating measurement time of the sample
%                   in seconds from application start
%    RPM.VALID    Nx1 array indicating the status of the position signal
%                   VALID >=  0: valid track and periodic signal
%                   VALID == -1: lost track or bad video signal
%                   VALID == -2: non-periodic breathing, e.g. coughing
%    RPM.TTLIN    Nx1 array indicating value of TTL input signal
%
%    If the file is DAT 1.4 or DAT 1.7, the following fields will also 
%    be present:
%
%    RPM.GATTYP  Either 'PHASE' for phase-based gating or 'AMP' for 
%                  amplitude based gating
%    RPM.GATWIN  2x2 array containing the gating window
%    RPM.VIDEO   Nx1 array of index of the video frame corresponding 
%                  to this position measurement (if video recording is enabled)
%    RPM.BEAM    Nx1 array indicating if treatment beam was enabled (for 
%                  treatment session)
%
%    If the file is a version VXP 1.5 or 1.6 file, the following fields 
%    will also be present:
%
%    RPM.MARK    Nx1 array that specifies sample when phase value is 
%                closest to 0 or Pi
%                  MARK == 0:  Neither 0 nor Pi phase
%                  MARK == 1:  Zero phase
%                  MARK == 2:  Pi phase
%    RPM.TTLOUT  Nx1 array indicating if CT scanner was triggered

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    Author:
%      Gregory C. Sharp
%      Massachusetts General Hospital
%      gcsharp@partners.org
%    History:
%      16-Jun-2005  GCS  1.0  - Initial version
%      17-Jun-2005  GCS  1.1  - Make loading faster by preallocating 
%                               output array using estimated file size
%      20-Dec-2005  GCS  1.2  - Add support for VXP 1.5 format
%      08-Apr-2008  GCS  1.3  - Rename vxp rpm.beam to rpm.ttlout to 
%                               match RPM documentation
%                             - Properly interpret scale factor in header
%      14-May-2008  GCS  1.4  - Update for DAT 1.7 format
%      23-Nov-2009  GCS  1.5  - Add Octave support
%      26-Nov-2009  GCS  1.5  - Update comments

rpm = [];
fp = fopen(fn,'r');
scale_factor = 1;     %% By default, DAT files are in cm units (?)

%% Use the first line to determine the version
s = deblank(fgetl(fp));
if (strcmp(s(1:3),'CRC'))
    rpm.version = 'DAT';
elseif (strcmp(s,'[Header]'))
    s = deblank(fgetl(fp));
    s = deblank(fgetl(fp));
    if (strcmp(s,'Version=1.5'))
        rpm.version = 'VXP 1.5';
    elseif (strcmp(s,'Version=1.6'))
        rpm.version = 'VXP 1.6';
    else
        error ('Unable to determine RPM file version (VXP version)');
    end
else
    error (['Unable to determine RPM file version (No header):' s]);
end

if (strcmp(rpm.version,'DAT'))
    %% Parse header for DAT 1.4, DAT 1.7
    s = deblank(fgetl(fp));
    [p,cnt] = sscanf(s,'VERSION = %s,',inf);
    rpm.version = ['DAT ', p];
    s = deblank(fgetl(fp));
    [p,cnt] = sscanf(s,'%g,',inf);
    if (cnt ~= 2)
        error(sprintf('Unexpected input in %s rpm file',rpm.version));
    end
    reference_position = p(1);
    s = deblank(fgetl(fp));
    if (~isempty(strfind(s,'#FALSE#')))
        rpm.gattyp = 'AMP';
    elseif (~isempty(strfind(s,'#TRUE#')))
        rpm.gattyp = 'PHASE';
    else
        error(sprintf('Unexpected input in %s rpm file',rpm.version));
    end
    s = deblank(fgetl(fp));
    [p,cnt] = sscanf(s,'%g,',inf);
    if (cnt ~= 2)
        error(sprintf('Unexpected input in %s rpm file',rpm.version));
    end
    rpm.gatwin = reference_position + p;
    for i=6:10
        deblank(fgetl(fp));
    end
elseif (strcmp(rpm.version,'VXP 1.5'))
    %% Parse header for VXP 1.5
    for i=4:9
        tmp = deblank(fgetl(fp));
        if (strncmp(tmp,'Scale_factor=',13))
            scale_factor = str2num(tmp(14:end)) / 10;
        end
    end
else
    %% Parse header for VXP 1.6
    for i=4:10
        tmp = deblank(fgetl(fp));
        if (strncmp(tmp,'Scale_factor=',13))
            scale_factor = str2num(tmp(14:end)) / 10;
        end
    end
end

%% Get size of file for preallocation
q = dir(fn);
fsize = q.bytes;
clear q;

%% Estimate pre-allocation size.  This estimate will be revised 
%% during the reading of the file
num_lines = ceil(fsize / 25);

%% Pre-allocate matrices
n = 0;
rpm_data = zeros(num_lines,7);

if (strcmp(rpm.version,'DAT 1.4'))
    %% <Signal_Value>,<Phase_Value>,<Timestamp>,<Valid_Flag>,<Video_Idx>,
    %%   <TTL_In>,<Beam_On>
    while (1)
        %% Store this line into p, break if end
	s = fgetl(fp);
        if ~ischar(s), break, end
        s = deblank(s);
        p = sscanf(s,'%g,',inf);
        
        %% Add p into rpm_data, extending if necessary
        n = n + 1;
        if (n > num_lines)
            disp('reallocating...');
            new_num_lines = num_lines + ceil((num_lines+1)/3);
            rpm_data = [rpm_data;zeros(new_num_lines,7)];
            num_lines = new_num_lines;
        end
        rpm_data(n,:) = p';
    end
    rpm_data = rpm_data(1:n,:);
    rpm.amp = rpm_data(:,1) * scale_factor;
    rpm.phase = rpm_data(:,2);
    rpm.time = rpm_data(:,3);
    rpm.valid = rpm_data(:,4);
    rpm.video = rpm_data(:,5);
    rpm.ttlin = rpm_data(:,6);
    rpm.beam = rpm_data(:,7);
elseif (strcmp(rpm.version,'DAT 1.7'))
    %% <Signal_Value>,<Phase_Value>,<Timestamp>,<Valid_Flag>,<Video_Idx>,
    %%   <TTL_In>,<Beam_On>
    while (1)
        %% Store this line into p, break if end
	s = fgetl(fp);
        if ~ischar(s), break, end
        s = deblank(s);
        if ~ischar(s), break, end
        p = sscanf(s,'%g,',inf);
        
        %% Trim extra fields
        p = [p(1);p(4:5);p(6:9)];
        
        %% Add p into rpm_data, extending if necessary
        n = n + 1;
        if (n > num_lines)
            disp('reallocating...');
            new_num_lines = num_lines + ceil((num_lines+1)/3);
            rpm_data = [rpm_data;zeros(new_num_lines,7)];
            num_lines = new_num_lines;
        end
        rpm_data(n,:) = p';
    end
    rpm_data = rpm_data(1:n,:);
    rpm.amp = rpm_data(:,1) * scale_factor;
    rpm.phase = rpm_data(:,2);
    rpm.time = rpm_data(:,3);
    rpm.valid = rpm_data(:,4);
    rpm.video = rpm_data(:,5);
    rpm.ttlin = rpm_data(:,6);
    rpm.beam = rpm_data(:,7);
else
    %% Parse VXP 1.5/1.6 version
    %% <Value_of_respiratory_wave><,><Phase_value><,><Time_Stamp><,>
    %%   <Valid_Flag><,><TTL_In><,><Mark><,><TTL_Out><cr>
    %% 
    %% VPX 1.5 does not have TTL_Out.  It looks like this:
    %%   -0.6249,3.0659,111424,0,1,
    %%   -0.6198,3.1169,111458,0,1,P
    %%   -0.6168,3.1916,111491,0,1,
    %% 
    while (1)

        %% Get line, break if end of file
	s = fgetl(fp);
        if ~ischar(s), break, end
        s = deblank(s);
        
        %% Convert line of text into numeric, store into variable "p"
        commas = findstr (s, ',');
        commas = [0, commas];
        p = zeros (1, 7);
        for i=1:length(commas)-1
            substring = s(commas(i)+1:commas(i+1)-1);
            val = str2num (substring);
            if (~isempty(val))
                p(i) = val;
            end
            %% Special processing for the 'P', 'Z' field
            if (i == 6 && strcmp (substring, 'Z'))
                p(i) = 1;
            elseif (i == 6 && strcmp (substring, 'P'))
                p(i) = 2;
            end
        end
        
        %% Add p into rpm_data, extending if necessary
        n = n + 1;
        if (n > num_lines)
            disp('reallocating...');
            new_num_lines = num_lines + ceil((num_lines+1)/3);
            rpm_data = [rpm_data;zeros(new_num_lines,7)];
            num_lines = new_num_lines;
        end
        rpm_data(n,:) = p';
    end
    
    rpm_data = rpm_data(1:n,:);
    rpm.amp = rpm_data(:,1) * scale_factor;
    rpm.phase = rpm_data(:,2);
    rpm.time = rpm_data(:,3);
    rpm.valid = rpm_data(:,4);
    rpm.ttlin = rpm_data(:,5);
    rpm.mark = rpm_data(:,6);
    rpm.ttlout = rpm_data(:,7);
end

rpm.time = rpm.time ./ 1000;

fclose(fp);
