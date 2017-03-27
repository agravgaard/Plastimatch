function MarkGroundTruth(path, prefix, fileExt, imageSize, pixelType, ...
    startFrame, minI, maxI, nClips)

fileFilter = [prefix, fileExt];
imageFiles = dir([path, fileFilter]);

nFrames = length(imageFiles);

resFile = [path, prefix, 'groundTruth.mat'];

% the index for the last frame
lastFrame = startFrame + nFrames - 1;

clipXY = zeros(nFrames, nClips * 2 + 1);
set(0,'Units','pixels')
scnSize = get(0,'ScreenSize');
h = figure('Position', scnSize);

disp('Left mouse button picks points.');
disp('Right mouse button picks last point.');
disp('clip ordering: scan from left to right, then top to bottom');

for iFrame=startFrame:lastFrame
    fid = fopen([path, imageFiles(iFrame).name], 'rb');
    curFrame = reshape(fread(fid, pixelType), imageSize);
    fclose(fid);
    curFrame = curFrame';
   
    imshow(curFrame, [minI, maxI], 'InitialMagnification','fit');
    
    title(['marking frame', ' ', num2str(iFrame), ' ', ...
        num2str(nFrames - iFrame), ' left']);
    
    hold on;
    xy = -ones (nClips, 2);    
    for iClip = 1:nClips
        [xi, yi, but] = ginput(1);
        if but == 1
            plot(xi, yi, 'c*');
            xy(iClip, :) = [xi, yi];
        else
            break;
        end
    end
    
    hold off;
    
    xy = xy';
    clipXY(iFrame, :) = [iFrame, xy(:)'];
    pause(0.1);
    clf;
end

disp('save marked to result file');
save(resFile, 'clipXY');


