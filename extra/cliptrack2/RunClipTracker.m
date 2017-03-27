% script to run ClipTracker
path = '\\physics.mgh.harvard.edu\IGRT_Research\fluoro\0001\0000a\';
prefix = '0_';
fileExt = '*.raw';
imageSize = [2048, 1536];
pixelType = 'ushort';
startFrame = 2;
nFrames = 39;
model = 'MI';
templateType = 'cylinder';
minI = 100;
maxI = 1200;
imageROI = [800 550 310 390];
nClips = 3;
nParticles = 500;
nIter = 5;
addpath('./fastop');

ClipTracker(path, prefix, fileExt, imageSize, pixelType, startFrame, nFrames, ...
    model, templateType, minI, maxI, imageROI, nClips, nParticles, ...
    nIter);