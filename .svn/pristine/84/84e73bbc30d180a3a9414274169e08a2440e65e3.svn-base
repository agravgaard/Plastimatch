function A = readviv(fn)

%% fn = 'c:/gsharp/nobackup/cache/iris/oneshot/i_0001.viv';

fp = fopen(fn,'r');
if (fp == -1)
  error ('Cannot open viv file for reading');
end

%% dd = dir('c:/gsharp/nobackup/cache/iris/oneshot/i_0000.viv');
dd = dir(fn);

A = [];
ext = fn(end-2:end);
if (strcmpi(ext,'viv'))
  isviv = 1;
elseif (strcmpi(ext,'raw'))
  isviv = 0;
else
  disp ('Not a raw file');
  return;
end

hires = 0;
if (dd.bytes >= 2048*1536*2)
  xres = 2048;
  yres = 1536;
else
  xres = 1024;
  yres = 768;
end

hdr_size = dd.bytes - xres*yres*2;

fseek(fp,hdr_size,'bof');
[A,count] = fread(fp,[xres,yres],'*uint16');
A = A';
fclose(fp);
