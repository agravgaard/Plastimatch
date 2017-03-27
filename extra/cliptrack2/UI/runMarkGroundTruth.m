% path = '\\physics.mgh.harvard.edu\IGRT_Research\fluoro\0001\0000a\';
% prefix = '0_';
% fileExt = '*.raw';
% imageSize = [2048, 1536];
% pixelType = 'ushort';
% startFrame = 1;
% minI = 0;
% maxI = 2600;
% nClips = 3;

path = '\\physics.mgh.harvard.edu\IGRT_Research\fluoro\0001\0000a\';
prefix = '1_';
fileExt = '*.raw';
imageSize = [2048, 1536];
pixelType = 'ushort';
startFrame = 1;
minI = 0;
maxI = 2600;
nClips = 3;

MarkGroundTruth(path, prefix, fileExt, imageSize, pixelType, ...
    startFrame, minI, maxI, nClips);