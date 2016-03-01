addpath('c:/gsharp/projects/fmatch-2/source-3/Release');

clear;close all;
parms = cyl_default_parms;
% parms.display_rate = 1;
prefix = 'piglung1';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002c';
imager = '0';
angle = 'lat';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-.7,444,1070];
parms.tracks(2).start_parms = [2,12,.7,830,1300];
parms.tracks(3).start_parms = [2,12,-.7,1262,1314];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002c';
imager = '1';
angle = 'ap';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-1.2,420,920];
parms.tracks(2).start_parms = [2,12,-.7,808,540];
parms.tracks(3).start_parms = [2,12,-1.2,1252,780];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002d';
imager = '0';
angle = 'lat';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-.7,440,1072];
parms.tracks(2).start_parms = [2,12,.7,824,1300];
parms.tracks(3).start_parms = [2,12,-.7,1272,1316];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002d';
imager = '1';
angle = 'ap';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-1.2,432,908];
parms.tracks(2).start_parms = [2,12,-.7,808,540];
parms.tracks(3).start_parms = [2,12,-1.2,1248,772];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002e';
imager = '0';
angle = 'lat';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-.7,448,1060];
parms.tracks(2).start_parms = [2,12,.7,828,1292];
parms.tracks(3).start_parms = [2,12,-.7,1260,1300];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002e';
imager = '1';
angle = 'ap';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-1.2,432,912];
parms.tracks(2).start_parms = [2,12,-.7,824,540];
parms.tracks(3).start_parms = [2,12,-1.2,1252,780];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002a';
imager = '0';
angle = 'lat';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-.7,448,1062];
parms.tracks(2).start_parms = [2,12,.7,832,1290];
parms.tracks(3).start_parms = [2,12,-.7,1262,1304];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002a';
imager = '1';
angle = 'ap';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-1.2,434,914];
parms.tracks(2).start_parms = [2,12,-.7,816,544];
parms.tracks(3).start_parms = [2,12,-1.2,1258,778];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002b';
imager = '0';
angle = 'lat';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-.7,434,1080];
parms.tracks(2).start_parms = [2,12,.7,826,1322];
parms.tracks(3).start_parms = [2,12,-.7,1262,1334];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
runname = '0002b';
imager = '1';
angle = 'ap';
clip_head = 0;
clip_tail = 0;

parms.tracks = [];
parms.tracks(1).start_parms = [2,12,-1.2,416,922];
parms.tracks(2).start_parms = [2,12,-.7,810,540];
parms.tracks(3).start_parms = [2,12,-1.2,1254,778];

indir = ['C:/gsharp/idata/iris-fluoro/',prefix,'/',runname,'/'];
inpat = [imager, '_*.raw'];
parms.first_frame = 1 + clip_head;
parms.last_frame = length(dir([indir,inpat]))-clip_tail;
parms = get_dirlist(parms,indir,inpat);
parms.out_file = [prefix, '_', runname, '_', angle, '.mat'];

results = clip_track(parms);

return;

