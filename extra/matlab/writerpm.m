function writerpm(filename,rpm)
% WRITERPM Write a Varian VXP 1.6 format file
%    writerpm (FILENAME, RPM)

[fp,error_msg] = fopen(filename,'w');
if (fp == -1)
  disp(['Cannot open file ', filename, ' for write: ', error_msg]);
  return;
end

if (isfield(rpm,'version'))
  if (~strcmp(rpm.version,'VXP 1.6'))
    disp(['Sorry, only VXP 1.6 is supported'])
  end
end

fprintf(fp,'[Header]\nCRC=0\nVersion=1.6\n');
fprintf(fp,'Data_layout=amplitude,phase,timestamp,validflag,ttlin,mark,ttlout\n');
fprintf(fp,'Patient_ID=Anonymous\n');
fprintf(fp,'Date=01-01-2000\n');
fprintf(fp,'Total_study_time=%.3g\n',rpm.time(end)-rpm.time(1));
fprintf(fp,'Samples_per_second=%g\n',(length(rpm.time)-1)/(rpm.time(end)-rpm.time(1)));
fprintf(fp,'Scale_factor=10\n');
fprintf(fp,'[Data]\n');

for i=1:length(rpm.time)
  if (rpm.mark(i) == 0)
    fprintf(fp,'%.4f,%.4f,%d,%d,%d,,%d\n',...
	    rpm.amp(i),rpm.phase(i),round(rpm.time(i)*1000),...
	    rpm.valid(i),rpm.ttlin(i),rpm.ttlout(i));
  elseif (rpm.mark(i) == 1)
    fprintf(fp,'%.4f,%.4f,%d,%d,%d,Z,%d\n',...
	    rpm.amp(i),rpm.phase(i),round(rpm.time(i)*1000),...
	    rpm.valid(i),rpm.ttlin(i),rpm.ttlout(i));
  elseif (rpm.mark(i) == 2)
    fprintf(fp,'%.4f,%.4f,%d,%d,%d,P,%d\n',...
	    rpm.amp(i),rpm.phase(i),round(rpm.time(i)*1000),...
	    rpm.valid(i),rpm.ttlin(i),rpm.ttlout(i));
  else
    disp('Warning: Unknown mark value %d at timestep %g\n',...
	 rpm.mark(i),rpm.time(i));
  end
end

fclose(fp);
