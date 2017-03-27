% =========================================================================
% function [track, state, linkLengths] = PropagateParticles(MODEL_PARAM, ...
%     state, curFrame, nClips, tSet, prevLinkLengths)
% 
% Compute weights based on matching cost  + link constaints and do mean 
% field Monte Carlo to propagate samples
% 
% Input:
% 
% Output:
% 
% Author: Rui Li
% Date: 09/20/2009
% =========================================================================
function [track, state, linkLengths] = PropagateParticles(MODEL_PARAM, ...
    state, curFrame, nClips, tSet, prevLinkLengths, nIterations)

% for numerical stability
eps = 1e-5;
beta = 0.1;

% allocate memory for track
% track(iClip, :) = [width, height, theta, rowidx, colidx]
track = zeros(nClips, 5);
% the template set
patterns = tSet.patCache;
widths = tSet.patWidths; nWidths = length(widths);
thetas = tSet.patThetas; nThetas = length(thetas);

% weight for each sample
pi_t = cell(nClips, nIterations);
% unweighted message
message = cell(nClips, 1);
% pairwise distance between samples
D = ComputeDistance(state);
[w, widthIdxs, thetaIdxs] = ComputeObsScore(curFrame, state, nClips, patterns, ...
    nWidths, nThetas);
[m, nSamples] = ComputeMessage(D, state, nClips, prevLinkLengths);

% the starting and end index for each clip
eIdx = cumsum(nSamples);
sIdx = cumsum(nSamples) + 1;
sIdx = [1, sIdx(1:end-1)];

% initialize the iteration
for iClip = 1:nClips
    % image likelihood
    w_i = w{iClip};
    % spatial constraints modeld as temporal consistency between
    % pair clip distances
    rowRange = sIdx(iClip):eIdx(iClip);
    colRange = [1:sIdx(iClip)-1, sIdx(iClip):eIdx(end)];
    m_i = m(rowRange, colRange);
    % store this message later weighted sum computation
    message{iClip} = m_i;
    m_i = sum(m_i, 2); 
    
    % compute joint weight and normalize the the weight
    pi_weight = exp(-beta * m_i) .* w_i;
    pi_t{iClip, 1} = pi_weight / (sum(pi_weight) + eps);
end

% this is the loop for message passing among the clips
for iIter = 2:nIterations
    pi_minus = cell2mat(pi_t);
    pi_minus = pi_minus(:, iIter - 1);
    for iClip=1:nClips
        % image likelihood
        w_i = w{iClip};
        % pairwise constraint 
        m_i = message{iClip};
        m_i = m_i (:, [1:sIdx(iClip)-1, eIdx(iClip)+1:eIdx(end)]);
        pi_minus_js = pi_minus([1:sIdx(iClip)-1, eIdx(iClip)+1:eIdx(end)]);
        m_i = m_i * pi_minus_js;
        
        % compute joint weight and normalize the the weight
        pi_weight = exp(- beta *m_i) .* w_i;
        pi_t{iClip, iIter} = pi_weight / sum(pi_weight);
    end
end

state_new = cell(nClips, 1);

for iClip = 1:nClips
    samples = [];
    % get the best matched width and rotataion index
    wIdxs = widthIdxs{iClip};
    tIdxs = thetaIdxs{iClip};
    
    pi_weight = pi_t{iClip, nIterations};
    cum_weight = cumsum(pi_weight);
    
    % track{iClip, :} = [width, height, theta, rowIdx, colIdx];
    [max_pi, estIdx] = max(pi_weight);
    % track contains the best estimate -- the sample with the highest
    % weight
    X = state{iClip};
    clipPos = double(X(:, end-1:end));
    track(iClip, :) = [widths(wIdxs(estIdx)) 2 thetas(tIdxs(estIdx)) clipPos(estIdx, :)];
    
    % linkLengths store the pair wise Euclidean distance between all clips
    % trick to get indices
    [i,j] = find(triu(ones(nClips), 1));
    % initialize output matrix
    linkLengths = zeros(nClips, nClips);
    linkLengths( i + nClips*(j-1) ) = ...
        sqrt(sum(abs( clipPos(i,:) - clipPos(j,:) ).^2, 2));
    linkLengths( j + nClips*(i-1) ) = linkLengths( i + nClips*(j-1) );


    % sample
    samp = rand(nSamples(iClip), 1);
    ind = BinarySearch(cum_weight, samp);
    bin = zeros(nSamples(iClip), 1);
    
    for i=1:nSamples(iClip)
        bin(ind(i)) = bin(ind(i)) + 1;
    end
    
    for i=find(bin>0)'
        samples = [samples; mvnrnd(clipPos(i,:), diag([16;49])*eye(2), bin(i))];
    end
    
    [newClipPos, uniqIdx] = unique(int32(samples), 'rows');
    wIdxs = wIdxs(uniqIdx);
    tIdxs = tIdxs(uniqIdx);
    state_new{iClip} = [wIdxs tIdxs newClipPos(:,1) newClipPos(:,2)];
end

state = state_new;



% -------------------------------------------------------------------------
function D = ComputeDistance(state)
% compute the messages in the form of pair wise distance
X = cell2mat(state);
% X now holds all the sampled locations for all the clips
X = double(X(:, end-1:end));
m = size(X,1);

% D is the distance matrix that holds the pair wise distance between
% all samples, but only the distances of samples from different clips
% will be used, memory is sacrificed for avoiding writing a loop to
% compute the pairwise distances between samples of different clips
[ i j ] = find(triu(ones(m), 1)); % trick to get indices
D = zeros(m, m); % initialise output matrix
D( i + m*(j-1) ) = sqrt(sum(abs( X(i,:) - X(j,:) ).^2, 2));
D( j + m*(i-1) ) = D( i + m*(j-1) );



% -------------------------------------------------------------------------
function [w wIdxs tIdxs] = ComputeObsScore(curFrame, states, nClips, patterns, ...
    nWidths, nThetas)

w = cell(nClips,1);
wIdxs = cell(nClips,1);
tIdxs = cell(nClips,1);

for iClip = 1:nClips
    X_i = states{iClip};
  
    % W_i is the image observation likelihood for the samples
    % for the iClip
    nLocations = size(X_i,1);
    
    % for each sample location, try different scale and rotation
    w_i = zeros(nLocations, 1);
    wIdx = zeros(nLocations, 1);
    tIdx = zeros(nLocations, 1);

    for lIdx = 1:nLocations
        widthIdx = X_i(lIdx, 1);
        % test two adjacent widths
        widthIdxs = widthIdx-1:widthIdx+1;
        widthIdxs = widthIdxs(widthIdxs >= 1);
        widthIdxs = widthIdxs(widthIdxs <= nWidths);
        
        thetaIdx = X_i(lIdx, 2);
        % test tow adjacent rotation angles
        thetaIdxs = thetaIdx-1:thetaIdx+1;
        thetaIdxs = thetaIdxs(thetaIdxs >= 1); 
        thetaIdxs = thetaIdxs(thetaIdxs <= nThetas);
                
        nTestWidths = length(widthIdxs);
        nTestThetas = length(thetaIdxs);
        
        w_is = zeros(nTestWidths*nTestThetas,1);
        wt_idxs = zeros(nTestWidths*nTestThetas,2);
        locRow = X_i(lIdx, 3); locCol = X_i(lIdx, 4);
        
        count = 1;
        for testWidthIdx = widthIdxs
            for testThetaIdx = thetaIdxs                
                T = patterns{testWidthIdx, testThetaIdx, 1};
                % rowOffset = offset(1); colOffset = offset(2)
                offset = patterns{testWidthIdx, testThetaIdx, 3};
                I = curFrame((locRow-offset(1)):(locRow+offset(1)), ...
                    (locCol-offset(2)):(locCol+offset(2)));
                % normalize W_i between in the range between 0 to 1
                w_is(count) = (prcorr2(T, I) + 1) / 2;
                wt_idxs(count, :) = [testWidthIdx, testThetaIdx];
                count = count + 1;
            end
        end
        % best match score at this location
        [w_i(lIdx), bestIdx] = max(w_is); 
        % clip width index of the best match
        wIdx(lIdx) = wt_idxs(bestIdx, 1); 
        % clip orientation of the best match
        tIdx(lIdx) = wt_idxs(bestIdx, 2); 
        
    end
    w{iClip} = w_i;
    wIdxs{iClip} = wIdx;
    tIdxs{iClip} = tIdx;
end  

% -------------------------------------------------------------------------    
% message is in the form of pairwise distance contraint
% the basic idea is that the distance between clips should not change
% too much in two adjacent time steps.
function [m, nSamplePos] = ComputeMessage(D, state, nClips, ...
    prevLinkLengths)

% array that stores the number of position samples per clip
nSamplePos = zeros(1,nClips);
% linkLenMat = zeros(size(D));
for iClip=1:nClips
    nSamplePos(iClip) = size(state{iClip}, 1);
end

% generate the linkLenMat which stores the col/row duplicated
% prevLinkLengths matrix so that effient distance measure between
% D and prevLinkLengths can be computed
p = cumsum(nSamplePos) + 1; % where does the next column start
idx = zeros(1, p(end)); % intialize
idx(p) = 1; % set the positions to 1
idx = idx(1:end-1); % ignore the last one
idx = cumsum(idx) + 1;
linkLenMat = prevLinkLengths(idx,idx);

m = (D - linkLenMat).^2;

% -------------------------------------------------------------------------
% 
% function index = BinarySearch(s, e)
% Input:  s - sorted vector
%         e - elements to be found
% Output: index - lowest inserting position
function index = BinarySearch(s, e)
index = zeros(size(e));

for i = 1 : size(e,1)*size(e,2)
    index(i) = oneBinarySearch(s,e(i));
end
% util function
function index = oneBinarySearch(s,e)

first = 1;
last = length(s);

if e > s(last)
    index = last + 1;
    return
end
if e < s(1)
    index = 1;
    return
end

while true
    mid = ceil((first + last) / 2);
    if s(mid-1) < e && e <= s(mid)
       index = mid;
       return;
    end
    if e <= s(mid-1) 
        last = mid - 1;
    else
        first = mid + 1;
    end
    if first >= last
        index = last;
        return;
    end
end



