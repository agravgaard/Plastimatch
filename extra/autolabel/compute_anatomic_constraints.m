
% Modified from computeminmaxG
% find out max and min gradients from the groundtruth data
fileList = dir('*.txt');
nFiles = length(fileList);
%gradinfo = zeros(nFiles, 2);
gradinfo = zeros(12,2);

for i=1:nFiles
   x = load(fileList(i).name);
   %grad = diff(x(:,2)) ./ diff(x(:,1));
   %grad = diff(x(:,1)) ./ diff(x(:,2));
   
   grad = [x(1:end-1,1),diff(x(:,1)) ./ diff(x(:,2))];
   
   %gradinfo(i,1) = min(grad);
   %gradinfo(i,2) = max(grad);
   if (i == 1)
       gradinfo(grad(:,1),1) = grad(:,2);
       gradinfo(grad(:,1),2) = grad(:,2);
   else
       gradinfo(grad(:,1),1) = min(gradinfo(grad(:,1),2), grad(:,2));
       gradinfo(grad(:,1),2) = max(gradinfo(grad(:,1),2), grad(:,2));
   end
end

%mingrad = min(gradinfo(:,1));
%maxgrad = max(gradinfo(:,2));

%save('gradstats.mat', 'mingrad', 'maxgrad', 'gradinfo');

