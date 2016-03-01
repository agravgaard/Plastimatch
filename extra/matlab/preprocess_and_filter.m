function preprocess_and_filter(parms)

indir = parms.indir;
tmpdir = parms.tmpdir;
outdir = parms.outdir;
I_0 = parms.I_0;
I_min = parms.I_min;
panel_no = parms.panel_no;

if (indir(end)~='/')
  indir = [indir, '/'];
end
if (tmpdir(end)~='/')
  tmpdir = [tmpdir, '/'];
end
if (outdir(end)~='/')
  outdir = [indir, '/'];
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
do_subsample = 1;
do_filter = 0;
use_medfilt = 0;
write_tmp_pfm = 1;
write_tmp_pgm = 1;
write_out_pfm = 1;
write_out_pgm = 1;
overwrite = 1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
mkdir(tmpdir);
mkdir(outdir);

d = dir([indir,'*.raw']);
rate = 0;
for i=1:1:length(d)
  tic();
  namebase = d(i).name;
  namebase = namebase(1:end-4);
  in_fn = [indir,d(i).name];
  tmp_pfm_fn = [tmpdir,namebase,'.pfm'];
  tmp_pgm_fn = [tmpdir,namebase,'.pgm'];
  out_pfm_fn = [outdir,namebase,'.pfm'];
  out_pgm_fn = [outdir,namebase,'.pgm'];
  
  if (overwrite == 0 ...
      && (~write_tmp_pgm || exist(tmp_pgm_fn)) ...
      && (~write_tmp_pfm || exist(tmp_pfm_fn)))
    continue;
  end
  
  disp(sprintf('LOADING %s [%d/%d, ETA: %.1f min]',in_fn,i,length(d),...
       rate*(length(d)-i+1)));
  A = double(readviv(in_fn));
  disp(sprintf('REPLACE BAD PIXELS'));
  B = replace_bad_pixels (A, panel_no);
  
  if (use_medfilt)
    B = medfilt2(A,[1,5]); B = medfilt2(A,[5,1]);
    C = double(A) - double(B);
    C(1:2,1:2) = 0;
    C(end-1:end,1:2) = 0;
    C(end-1:end,end-1:end) = 0;
    C(1:2,end-1:end) = 0;
    B(abs(C) < 100) = A(abs(C) < 100);
  end
  
  %% Subsample
  if (do_subsample)
    disp(sprintf('SUBSAMPLE'));
    B = (B(1:2:end,1:2:end) + B(1:2:end,2:2:end) + ...
	 B(2:2:end,1:2:end) + B(2:2:end,2:2:end)) / 4;
  end
  
  %% GCS: Aug 29, 2006
  disp(sprintf('FIX INTENSITIES'));
  B(B>I_0) = I_0;
  B(B<I_min) = I_min;
  B = B / I_0;
  B = - log(B);
  
  %% Filter -- doesn't work at this time
  %% if (do_filter)
  %%   B = 0.5 * (B + prefilt(double(B),k1,k2));
  %% end

  if (write_tmp_pgm)
    disp(sprintf('SAVE PGM %s',tmp_pgm_fn));
    savepgm(B*10000,tmp_pgm_fn,1,0);
  end
  if (write_tmp_pfm)
    disp(sprintf('SAVE PFM %s',tmp_pfm_fn));
    savepfm (B,tmp_pfm_fn);
  end
  
  if (overwrite == 0 ...
      && (~write_out_pgm || exist(out_pgm_fn)) ...
      && (~write_out_pfm || exist(out_pfm_fn)))
    continue;
  end
  
  disp(sprintf('FILTER'));
  B = prefilt(B);
  if (write_out_pgm)
    pgm_min = -2;
    pgm_max = 3.5;
    B1 = (B + pgm_min) / (pgm_max - pgm_min);
    B1(B1<0) = 0;
    B1(B1>1) = 1;
    disp(sprintf('SAVE PGM %s',out_pgm_fn));
    savepgm (B1*255,out_pgm_fn,1,255);
  end
  if (write_out_pfm)
    disp(sprintf('SAVE PFM %s',out_pfm_fn));
    savepfm (B,out_pfm_fn);
  end
  rate = toc() / 60;
end
