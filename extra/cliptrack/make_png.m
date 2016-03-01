function make_png (results,frame_no,png_out)

frames = results.first_frame:results.last_frame;
si=frame_no;

fn = results.dirlist{frames(frame_no)};
if (strcmp(fn(end-3:end),'.viv') || strcmp(fn(end-3:end),'.raw'))
  B = double(readviv(fn));
else
  B = double(imread(fn));
end

B = medfilt2(B,[5 5]);
  
Bs = (B(1:2:end-1,1:2:end-1) + B(1:2:end-1,2:2:end) ...
      + B(2:2:end,1:2:end-1) + B(2:2:end,2:2:end)) / 4;
Bs = (Bs(1:2:end-1,1:2:end-1) + Bs(1:2:end-1,2:2:end) ...
      + Bs(2:2:end,1:2:end-1) + Bs(2:2:end,2:2:end)) / 4;
Bs = my_histeq_uniform (Bs);
  
imwrite(Bs/255,sprintf(png_out,frames(si)));
