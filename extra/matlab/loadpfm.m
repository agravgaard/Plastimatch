function A = loadpfm(filename)
%%  A = loadpfm(filename)

fp = fopen(filename,'r');

%% Skip 1 line
fgetl(fp);
cols = fscanf (fp, '%d', 1);
rows = fscanf (fp, '%d', 1);
fgetl(fp);
%% Skip 1 line
fgetl(fp);

[A,count] = fread(fp,rows*cols,'*float');
A = double(A);
% A = reshape(A,rows,cols);
A = reshape(A,cols,rows);
A = A';

fclose (fp);
