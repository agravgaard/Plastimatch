#cd ([getenv('HOME'), '/Dropbox/autolabel']);
#cd ([getenv('HOME'), '/My Dropbox/autolabel']);
cd ([getenv('HOME'), '/data/autolabel']);

training_dir = 't-spine-2-train';

data = [];
pid = [];
sid = [];

d = dir ([training_dir, '/*.raw']);
for i = 1:length(d)
    pid(i) = str2num(d(i).name(1:2));
    j = max(findstr(d(i).name,'_'));
    k = max(findstr(d(i).name,'.'));
    sid(i) = str2num(d(i).name(j+2:k-1));
    fp = fopen (["t-spine-2/", d(i).name]);
    data(i,:) = fread (fp, Inf, "float32");
    fclose (fp);
end

fp = fopen ([training_dir, '/t-spine.libsvm'], "w");
for i = 1:length(sid)
    fprintf (fp, "%d", sid(i));
    for j = 1:size(data,2)
        fprintf (fp, " %d:%f", j-1, data(i,j));
    end
    fprintf (fp, "\n");
end
fclose (fp);
