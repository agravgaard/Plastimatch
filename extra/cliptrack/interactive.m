function interactive_results = interactive(B,rowcol_cur,parms)

num_tracks = size(rowcol_cur,1);
ws = parms.ws;
ss = parms.ss;

for i=1:num_tracks
  rowcol = rowcol_cur(i,:);
  rows = rowcol(1)-ws-ss:rowcol(1)+ws+ss;
  cols = rowcol(2)-ws-ss:rowcol(2)+ws+ss;
  if (rows(1) <= 0)
    rows = rows - rows(1) + 1;
  elseif (rows(end) > size(B,1))
    rows = rows + size(B,1) - rows(end);
  end
  if (cols(1) <= 0)
    cols = cols - cols(1) + 1;
  elseif (cols(end) > size(B,2))
    cols = cols + size(B,2) - cols(end);
  end
  Btmp = B(rows,cols);
  dsp(Btmp,1);
  [c,r] = ginput(1);
  interactive_results(i,:) = [r,c] + [rows(1),cols(1)] - [1,1];
end
