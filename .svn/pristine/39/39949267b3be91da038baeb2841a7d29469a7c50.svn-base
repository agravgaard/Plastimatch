dirname = "rider-lung";
d = dir ([dirname, "/*.fcsv"]);

for i=1:size(d)
    fn = [dirname, "/", d(i).name];
    [locs, labels] = readfcsv(fn);
    LLA = find(strcmp(labels,"LLA"));
    LLD = find(strcmp(labels,"LLD"));
    RLA = find(strcmp(labels,"RLA"));
    RLD = find(strcmp(labels,"RLD"));
    CAR = find(strcmp(labels,"Car"));
    idxs = [1:size(labels,2)];
    idxs = complement ([LLD,LLA,CAR,RLA,RLD], idxs);
    
    [x,ix1] = min(abs(locs(idxs,3) - locs(LLA,3)));
    [x,ix2] = min(abs(locs(idxs,3) - locs(RLA,3)));
    [x,ix3] = min(abs(locs(idxs,3) - locs(CAR,3)));
    [x,ix4] = min(abs(locs(idxs,3) - locs(LLD,3)));
    [x,ix5] = min(abs(locs(idxs,3) - locs(RLD,3)));
    disp ([fn,",",labels{ix1},",",labels{ix2},",",labels{ix3},",",...
           labels{ix4},",",labels{ix5}]);
end
