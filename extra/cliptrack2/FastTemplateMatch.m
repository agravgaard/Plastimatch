function score = FastTemplateMatch(I, T, iSize, tSize)

%output image size
oSize = iSize + tSize - 1;

% compute correlation in frequency domain
FT = fft2(rot90(T,2), oSize(1), oSize(2));
FI = fft2(I, oSize(1), oSize(2));
score = real(ifft2(FI.*FT));

% compute local sum and local quadratic sum of I
iLocalSum1 = ComputeLocalSum(I, tSize);
iLocalSum2 = ComputeLocalSum(I.*I, tSize);

% standard deviation 
iStd = sqrt(max(iLocalSum2-(iLocalSum1.^2)/numel(T),0) );
tStd = sqrt(numel(T)-1)*std(T(:));

% mean compensation
meanIT = iLocalSum1 * sum(T(:))/numel(T);
score = 0.5 + (score-meanIT)./ ( 2 * tStd * max(iStd, tStd/1e5));

score = UnpadArray(score, iSize);

% -------------------------------------------------------------------------
% utitlity functions
function B = UnpadArray(A,Bsize)
Bstart=ceil((size(A)-Bsize)/2)+1;
Bend=Bstart+Bsize-1;
if(ndims(A)==2)
    B=A(Bstart(1):Bend(1),Bstart(2):Bend(2));
elseif(ndims(A)==3)
    B=A(Bstart(1):Bend(1),Bstart(2):Bend(2),Bstart(3):Bend(3));
end
    
function localSum = ComputeLocalSum(I,tSize)
% Add padding to the image
B = padarray(I,tSize);

% Calculate for each pixel the sum of the region around it,
% with the region the size of the template.
s = cumsum(B,1);
c = s(1+tSize(1):end-1,:)-s(1:end-tSize(1)-1,:);
s = cumsum(c,2);
localSum= s(:,1+tSize(2):end-1)-s(:,1:end-tSize(2)-1);
% -------------------------------------------------------------------------
