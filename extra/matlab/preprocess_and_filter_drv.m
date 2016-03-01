%% I_0 is the intensity of unattenuated pixels (no attenuation)
%% I_min is the minimum allowed intensity (full attenuation)
%% panel_no is used for bad pixel replacement


parms.indir = 'c:/gsharp/idata/gpuit-data/2006-05-26/0005a/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2006-05-26/0005a-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2006-05-26/0005a-small-filt/';
parms.I_0 = 8800;
I_min = 500;
parms.panel_no = 2;

parms.indir = 'c:/gsharp/idata/gpuit-data/2006-05-26/0005c/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2006-05-26/0005c-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2006-05-26/0005c-small-filt/';
I_0 = 1900;
I_min = 100;
parms.panel_no = 2;

parms.indir = 'c:/gsharp/idata/gpuit-data/2006-12-11/0000b/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2006-12-11/0000b-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2006-12-11/0000b-small-filt/';
I_0 = 8100;
I_min = 30;
parms.panel_no = 2;

parms.indir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0002/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0002-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0002-small-filt/';
%I_0 = 10300;   %% This is what I tried before.  Which setting is better?
%I_min = 100;
parms.I_0 = 8800;
parms.I_min = 500;
parms.panel_no = 2;

parms.indir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0003/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0003-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2007-02-08/0003-small-filt/';
parms.I_0 = 8800;
parms.I_min = 500;
parms.panel_no = 2;

parms.indir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a-small-filt/';
parms.I_0 = 4320;
parms.I_min = 5;
parms.panel_no = 2;

parms.indir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a/';
parms.tmpdir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a-small/';
parms.outdir = 'c:/gsharp/idata/gpuit-data/2006-09-12/0001a-small-filt/';
parms.I_0 = 4320;
parms.I_min = 5;
parms.panel_no = 2;

parms.indir = 'g:/reality/2008-03-13/0000/';
parms.tmpdir = 'g:/reality/2008-03-13/0000-small/';
parms.outdir = 'g:/reality/2008-03-13/0000-small-filt/';
parms.I_0 = 8800;
parms.I_min = 5;
parms.panel_no = 1;

preprocess_and_filter(parms);
