function DrawParticles(nClips, state)

gcf; hold on;
colorSpecs = ['y','m','c','r','g','b','w','k'];

for iClip = 1:nClips
    plot(state{iClip}(:, 4),state{iClip}(:,3), [colorSpecs(iClip) '*']);
end