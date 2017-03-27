function ShowAllHypothese(state, nClips, widths, thetas)
gcf; hold on;
colorSpecs = ['y','m','c','r','g','b','w','k'];
lineWidth = 1;
for iClip =1:nClips
    X_i = double(state{iClip});
    nSamples = size(X_i,1);
    for iSample = 1:nSamples
        DrawRectangle...
            (widths(X_i(iSample, 1)), 2, thetas(X_i(iSample, 2)), ...
            X_i(iSample, 3), ...
            X_i(iSample, 4), colorSpecs(iClip), lineWidth);
    end
end

hold off;