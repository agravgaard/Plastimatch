function [locs,labels] = readfcsv(fn)
%% Usage: [locs,labels] = readfcsv(fn)

fp = fopen(fn,'rb');
if (fp == -1)
  error ('Cannot open fcsv file for reading');
end

locs = [];
labels = [];
i = 1;
while (1)
  t = fgetl(fp);
  if (t == -1)
      break;
  end
  if (length(t) == 0)
      continue;
  end
  if (t(1) == '#')
      continue;
  end

  %% Parse line
  [v1, count] = sscanf (t, '%[^,],%f,%f,%f,%d,%d');
  if (count != 6)
      disp(sprintf('Error parsing line %s',t))
      continue;
  end
  labels{i} = char(v1(1:end-5)');
  locs(i,:) = v1(end-4:end-2);
  %% (Convert from RAS to LPS)
  locs(i,1:2) = - locs(i,1:2);
  i = i + 1;
end

fclose(fp);

return;
