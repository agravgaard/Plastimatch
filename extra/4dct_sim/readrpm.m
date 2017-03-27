function rpm = readrpm(fn)
% READRPM Read Varian RPM format file
%    RPM = READRPM(FN) reads the information from the RPM file FN, which 
%    can be in either DAT 1.4, VXP 1.5, or VXP 1.6 format.  
%    The contents of the file are returned in the structure RPM, which 
%    has the following fields:
%
%    RPM.TYPE    String indicating the file type.  Valid values are 
%                'DAT 1.4', 'VXP 1.5', or 'VXP 1.6'
%
%    The following fields will be Nx1 arrays, with one element per 
%    data point:
%
%    RPM.AMP     Position in centimeters relative to an arbitrary reference
%    RPM.PHASE   Phase value for this sample
%    RPM.TIME    Time of the sample measurement in seconds from 
%                application start
%    RPM.VALID   Flag indicating the status of the position signal
%                  VALID >=  0: valid track and periodic signal
%                  VALID == -1: lost track or bad video signal
%                  VALID == -2: non-periodic breathing, e.g. coughing
%    RPM.TTLIN   Flag indicating value of TTL input signal
%    RPM.BEAM    Flag indicating if treatment beam was enabled (for 
%                treatment session), or if CT scanner was triggered 
%                (for CT session)
%
%    If the file is a version DAT 1.4 file, the following fields 
%    will be present:
%
%    RPM.GATTYP  Either 'PHASE' for phase-based gating or 'AMP' for 
%                amplitude based gating
%    RPM.GATWIN  2x2 array containing the gating window
%    RPM.VIDEO   Nx1 array of index of the video frame corresponding 
%                to this position measurement (if video recording is enabled)
%
%    If the file is a version VXP 1.6 file, the following field will be 
%    present:
%
%    RPM.MARK    Nx1 array that specifies sample when phase value is 
%                closest to 0 or Pi
%                  MARK == 0:  Neither 0 nor Pi phase
%                  MARK == 1:  Zero phase
%                  MARK == 2:  Pi phase

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    Author:
%      Gregory C. Sharp
%      Massachusetts General Hospital
%      gcsharp@partners.org
%    History:
%      16-Jun-2005  GCS  1.0  Initial version
%      17-Jun-2005  GCS  1.1  Make loading faster by preallocating 
%                             output array using estimated file size
%      20-Dec-2005  GCS  1.2  Add support for VXP 1.5 format

rpm = [];
fp = fopen(fn,'r');

%% Use the first line to determine the version
s = fgetl(fp);
if (strcmp(s(1:3),'CRC'))
  rpm.version = 'DAT 1.4';
elseif (strcmp(s,'[Header]'))
  s = fgetl(fp);
  s = fgetl(fp);
  if (strcmp(s,'Version=1.5'))
    rpm.version = 'VXP 1.5';
  elseif (strcmp(s,'Version=1.6'))
    rpm.version = 'VXP 1.6';
  else
    error ('Unable to determine RPM file version');
  end
else
  error ('Unable to determine RPM file version');
end

if (strcmp(rpm.version,'DAT 1.4'))
  %% Parse header for DAT 1.4
  for i=2
    fgetl(fp);
  end
  s = fgetl(fp);
  [p,cnt,errmsg,ni] = sscanf(s,'%g,',inf);
  if (cnt ~= 2)
    error ('Unexpected input in DAT 1.4 rpm file');
  end
  reference_position = p(1);
  s = fgetl(fp);
  if (~isempty(strfind(s,'#FALSE#')))
    rpm.gattyp = 'AMP';
  elseif (~isempty(strfind(s,'#TRUE#')))
    rpm.gattyp = 'PHASE';
  else
    error ('Unexpected input in DAT 1.4 rpm file');
  end
  s = fgetl(fp);
  [p,cnt,errmsg,ni] = sscanf(s,'%g,',inf);
  if (cnt ~= 2)
    error ('Unexpected input in DAT 1.4 rpm file');
  end
  rpm.gatwin = reference_position + p;
  for i=6:10
    fgetl(fp);
  end
elseif (strcmp(rpm.version,'VXP 1.5'))
  %% Parse header for VXP 1.5
  for i=4:9
    fgetl(fp);
  end
else
  %% Parse header for VXP 1.6
  for i=4:10
    fgetl(fp);
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
  %% Parse DAT 1.4 version
  %% <Signal_Value>,<Phase_Value>,<Timestamp>,<Valid_Flag>,<Video_Idx>,
  %%   <TTL_In>,<Beam_On>
  while (1)
    %% Store this line into p, break if end
    s = fgetl(fp);
    if ~ischar(s), break, end
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
  rpm.amp = rpm_data(:,1);
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
  %% TTL_OUT is only in VPX 1.6.
  while (1)
    %% Store line of text into p, break if end of file
    s = fgetl(fp);
    if ~ischar(s), break, end
    [p,cnt,errmsg,ni] = sscanf(s,'%g,',inf);
    if (cnt ~= 5)
      error ('Unexpected input in VXP 1.5/1.6 rpm file');
    end
    if (ni <= length(s))
      switch(s(ni))
       case ','
	p(6) = 0;
	s = s(ni+1:end);
       case 'Z'
	p(6) = 1;
	s = s(ni+2:end);
       case 'P'
	p(6) = 2;
	s = s(ni+2:end);
       otherwise
      end
    else
      p(6) = 0;
    end
    if (strcmp(rpm.version,'VXP 1.5'))
      p(7) = 0;
    else
      p(7) = str2num(s);
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
  rpm.amp = rpm_data(:,1);
  rpm.phase = rpm_data(:,2);
  rpm.time = rpm_data(:,3);
  rpm.valid = rpm_data(:,4);
  rpm.ttlin = rpm_data(:,5);
  rpm.mark = rpm_data(:,6);
  rpm.beam = rpm_data(:,7);
end

rpm.time = rpm.time ./ 1000;

fclose(fp);
