function writeFig(figNo, fileName, resolution)
% make the backgroung white
set(figNo, 'color', 'w');
f = getframe(figNo);
colormap(f.colormap);
imwrite(f.cdata, fileName, 'Resolution', resolution);

% Or:
% print -f1 -r500 -dtiff filename