clear;close all;
parms = circ_default_parms;
parms.display_rate = 5;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
indir = 'C:/gsharp/idata/iris-fluoro/gating/0003/';
inpat = '0_*.raw';
parms.first_frame = 1;
parms.last_frame = length(dir([indir,inpat]));
parms = get_dirlist(parms,indir,inpat);
parms.tracks = [];
parms.out_file = 'gating_0003.mat';

parms.tracks(1).start_parms = [872,735];

results = clip_track(parms);

return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
indir = 'C:/gsharp/idata/iris-fluoro/day3/0000a/';
inpat = '0_*.raw';
parms.first_frame = 1;
parms.last_frame = length(dir([indir,inpat]));
parms = get_dirlist(parms,indir,inpat);
parms.tracks = [];
parms.out_file = 'day3_0000a.mat';

parms.tracks(1).start_parms = [1023,1139];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
indir = 'C:/gsharp/idata/iris-fluoro/day3/0000b/';
inpat = '0_*.raw';
parms.first_frame = 1;
parms.last_frame = length(dir([indir,inpat]));
parms = get_dirlist(parms,indir,inpat);
parms.tracks = [];
parms.out_file = 'day3_0000b.mat';

parms.tracks(1).start_parms = [1023,1139];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
indir = 'C:/gsharp/idata/iris-fluoro/day3/0000c/';
inpat = '0_*.raw';
parms.first_frame = 1;
parms.last_frame = length(dir([indir,inpat]));
parms = get_dirlist(parms,indir,inpat);
parms.tracks = [];
parms.out_file = 'day3_0000c.mat';

parms.tracks(1).start_parms = [1023,1161];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
indir = 'C:/gsharp/idata/iris-fluoro/day3/0000d/';
inpat = '0_*.raw';
parms.first_frame = 1;
parms.last_frame = length(dir([indir,inpat]));
parms = get_dirlist(parms,indir,inpat);
parms.tracks = [];
parms.out_file = 'day3_0000d.mat';

parms.tracks(1).start_parms = [1024,1138];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
indir = 'C:/gsharp/idata/iris-fluoro/day3/0000e/';
inpat = '0_*.raw';
parms.first_frame = 1;
parms.last_frame = length(dir([indir,inpat]));
parms = get_dirlist(parms,indir,inpat);
parms.tracks = [];
parms.out_file = 'day3_0000e.mat';

parms.tracks(1).start_parms = [1023,1173];

results = clip_track(parms);
