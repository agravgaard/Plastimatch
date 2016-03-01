function B = replace_bad_pixels (A, panel_no)

badpix = load(sprintf('badpix_%d.txt',panel_no));
B = A;

for i=1:size(badpix,1)
  r = badpix(i,1):badpix(i,2);
  cv = (A(r,badpix(i,3)-1) + A(r,badpix(i,4)+1)) / 2;
  for c = badpix(i,3):badpix(i,4);
    B(r,c) = cv;
  end
end
