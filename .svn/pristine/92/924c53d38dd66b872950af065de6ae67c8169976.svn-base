indir = 'c:/gsharp/idata/gpuit-data/2006-12-11/0000b/';
indir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0002/';
indir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a/';
indir = 'g:/reality/2008-03-13/0000/';
inpat = '*.raw';
parms = [];
d = dir([indir,'/',inpat]);
ss = 10;  %% Subsampling rate

namebase = d(1).name;
frame(1) = str2num(namebase(3:8));
prev = double(readviv([indir, d(1).name]));
prev = prev(1:ss:end,1:ss:end);
val(1) = median(prev(:));
%% for i=2:length(d)
for i=length(d)-100:length(d)
  if (mod(i,10)==0), disp(sprintf('%4d',i)), end
  frame(i) = str2num(namebase(3:8));
  curr = double(readviv([indir, d(i).name]));
  curr = curr(1:ss:end,1:ss:end);
  namebase = d(i).name;
  frame(i) = str2num(namebase(3:8));
  absdiff(i) = median(abs(curr(:) - prev(:)));
  diff2(i) = median((curr(:) - prev(:)).^2);
  val(i) = median(curr(:));
  prev = curr;
end
for i=2:length(d)
  namebase = d(i).name;
  frame(i) = str2num(namebase(3:8));
end

figure(1);clf;
subplot(2,2,1);
plot(frame,absdiff,'b.-');
subplot(2,2,2);
plot(frame,diff2,'r.-');
subplot(2,2,3);
plot(frame,val,'g.-');
