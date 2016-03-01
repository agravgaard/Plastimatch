function make_avi (results,avi_out,options)

%% Options:
%%   options.subwin
%%   options.first_frame
%%   options.last_frame
%%   options.use_figure
%%   options.draw_tracks

if (isempty(avi_out))
  avi_out = 'out.avi';
end

if (exist(avi_out,'file') == 2)
  clear functions;
  delete(avi_out);
end

if (~isfield(options,'compression'))
  options.compression = 'uncompressed';
end
if (~isfield(options,'first_frame'))
  options.first_frame = results.first_frame;
end
if (~isfield(options,'last_frame'))
  options.last_frame = results.last_frame;
end
if (~isfield(options,'winlev'))
  options.winlev = 'uniform';
end


switch lower(options.compression)
 case 'uncompressed'
  mov_out = avifile (avi_out,'fps',6.0,'quality',80,'compression','None','colormap',gray(256));
  %mov_out = avifile (avi_out,'fps',6.0,'compression','None');
 case 'cinepak'
  %mov_out = avifile (avi_out,'fps',6.0,'quality',80,'compression','Cinepak');
  mov_out = avifile (avi_out,'fps',6.0,'quality',80,'compression','Cinepak','colormap',gray(256));
 case 'indeo3'
  mov_out = avifile (avi_out,'fps',6.0,'quality',80,'compression','Indeo3');
 case 'indeo5'
  %mov_out = avifile (avi_out,'fps',6.0,'quality',80,'compression','Indeo5');
  mov_out = avifile (avi_out,'fps',6.0,'quality',80,'compression','Indeo5','colormap',gray(236));
end

frames = options.first_frame:options.last_frame;
frames = frames - results.first_frame + 1;
for si=1:length(frames)
  printf ('---- FRAME %d', si);

  fn = results.dirlist{frames(si)+results.first_frame-1};
  if (strcmp(fn(end-3:end),'.viv') || strcmp(fn(end-3:end),'.raw'))
    B = double(readviv(fn));
  else
    B = double(imread(fn));
  end
  wloc = [1 size(B,1) 1 size(B,2)];

  if (isfield(options,'subwin'))
    s = options.subwin;
    B = B(s(1):s(2),s(3):s(4));
    wloc = options.subwin;
  end
  
  B = medfilt2(B,[5 5]);

  %% B = flipud(B');

  Bs = B;
  if (isfield(options,'subsample'))
    for i=1:options.subsample
      Bs = (Bs(1:2:end-1,1:2:end-1) + Bs(1:2:end-1,2:2:end) ...
	    + Bs(2:2:end,1:2:end-1) + Bs(2:2:end,2:2:end)) / 4;
    end
  end
  
  switch lower(options.winlev)
   case 'uniform'
    Bs = my_histeq_uniform (Bs);
   case 'bottop'
    w = options.bottop;
    Bs = (Bs - w(1)) / (w(2) - w(1));
    Bs = max(min(Bs,1),0) * 255;
   case 'normalized'
    Bs = imnorm (Bs);
  end

  if (isfield(options,'use_figure') && options.use_figure)
    close all;
    image(Bs);
    colormap(gray(256));
    hold on;

    if (isfield(options,'draw_tracks'))
      for i=options.draw_tracks
	%% This is not quite right
	pos = results.tracks(i).rowcol(frames(si),:);
	pos = pos - wloc([1,3]);
	sq = 10;
	px = pos(2) + [-sq,+sq,+sq,-sq,-sq];
	py = pos(1) + [-sq,-sq,+sq,+sq,-sq];
	plot(px,py,'r');
      end
    end

    frame = getframe(gca);
    drawnow;
    mov_out = addframe(mov_out,frame);
  else

    clear C;
    %Bcol(:,:,1) = Bs/255;
    %Bcol(:,:,2) = Bs/255;
    %Bcol(:,:,3) = Bs/255;
    %Bcol = Bcol * 235 / 255;
    C = Bs;

    if (isfield(options,'draw_tracks'))
      for i=options.draw_tracks
	%% This is not quite right
	pos = results.tracks(i).rowcol(frames(si),:);
	pos = pos - wloc([1,3]);
	sq = 10;
	ix = pos(2) + [-sq,+sq;
		       +sq,+sq;
		       -sq,+sq;
		       -sq,-sq];
	iy = pos(1) + [-sq,-sq;
		       -sq,+sq;
		       +sq,+sq;
		       -sq,+sq];
	ox = pos(2) + [-sq-1,+sq+1;
		       +sq+1,+sq+1;
		       -sq-1,+sq+1;
		       -sq-1,-sq-1];
	oy = pos(1) + [-sq-1,-sq-1;
		       -sq-1,+sq+1;
		       +sq+1,+sq+1;
		       -sq-1,+sq+1];
	
	ocol = 255.0;
	icol = 0.0;

	for bb=1:4
	  i1c = max(1,ix(bb,1)):min(size(C,2),ix(bb,2));
	  i1r = max(1,iy(bb,1)):min(size(C,2),iy(bb,2));
	  o1c = max(1,ox(bb,1)):min(size(C,2),ox(bb,2));
	  o1r = max(1,oy(bb,1)):min(size(C,2),oy(bb,2));
	  C(i1r,i1c) = icol;
	  C(o1r,o1c) = ocol;
	end
	
      end
    end

    mov_out = addframe(mov_out,C);
  end
end

mov_out = close(mov_out);
