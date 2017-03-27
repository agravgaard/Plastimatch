function savepfm(A,filename)
%%   savepfm(A,filename)

fp = fopen(filename,'w');

%% No comments in PFM format
fprintf (fp,'Pf\n');
fprintf (fp,'%d %d\n',size(A,2),size(A,1));
%% See http://netpbm.sourceforge.net/doc/pfm.html
fprintf (fp,'-1\n');

A = A';
fwrite (fp,A,'real*4');

fclose (fp);
