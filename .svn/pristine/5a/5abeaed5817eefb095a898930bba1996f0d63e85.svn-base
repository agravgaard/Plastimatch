function parms = get_dirlist(parms,indir,inpat)

d = dir([indir,inpat]);
parms.dirlist = [];
parms.timestamp = [];
parms.frame_no = [];
for i=1:length(d)
  parms.dirlist{i} = [indir,d(i).name];
  parms.timestamp(i) = str2num(d(i).name(10:23));
  parms.frame_no(i) = str2num(d(i).name(3:8));
end
