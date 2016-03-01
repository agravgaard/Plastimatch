function writenrrd(fn,A,offset,spacing,type)
%% writenrrd(fn,A,offset,spacing,type)
%%   fn        filename
%%   A         Volume
%%   offset
%%   spacing
%%   type      'uchar','float', or 'short'

Asz = size(A);

switch ndims(A)
 case 3,
 case 4,
  error ('Sorry, 4D (VF) volume not yet supported');
 otherwise
  error ('Sorry, only 3D & VF volumes supported');
end

fp = fopen(fn,'wb');
if (fp == -1)
  error ('Cannot open nrrd file for writing');
end

fprintf (fp,'NRRD0002\n');
fprintf (fp,'dimension: 3\n');
fprintf (fp,'encoding: raw\n');
fprintf (fp,'endian: little\n');
fprintf (fp,'sizes:');
fprintf (fp,' %d',Asz(1:3));
fprintf (fp,'\n');
fprintf (fp,'spacings:');
fprintf (fp,' %g',spacing);
fprintf (fp,'\n');
%% The NRRD format is strange.  Apparently this should be "node", 
%% not "cell".
fprintf (fp,'centers: node node node\n');
fprintf (fp,'axismins:');
fprintf (fp,' %g',offset);
fprintf (fp,'\n');

%% This works for 3D (A stays the same), and 4D (shift left 1)
A = shiftdim(A,3);

switch(lower(type))
 case 'uchar'
  fprintf (fp,'type: uchar\n');
  fprintf (fp,'\n');
  fwrite (fp,A,'uint8');
 case 'short'
  fprintf (fp,'type: short\n');
  fprintf (fp,'\n');
  fwrite (fp,A,'int16');
 case 'float'
  fprintf (fp,'type: float\n');
  fprintf (fp,'\n');
  %% GCS: 'real4' doesn't work on matlab 2006
  fwrite (fp,A,'real*4');
 otherwise
  fclose(fp);
  error ('Sorry, unsupported type');
end

fclose(fp);
