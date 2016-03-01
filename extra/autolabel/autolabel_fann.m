cd ([getenv('HOME'), '/Dropbox/autolabel']);

data = [];
pid = [];
sid = [];

d = dir ("t-spine/*.raw");
for i = 1:length(d)
    pid(i) = str2num(d(i).name(1:2));
    j = max(findstr(d(i).name,'_'));
    k = max(findstr(d(i).name,'.'));
    sid(i) = str2num(d(i).name(j+1:k-1));
    fp = fopen (["t-spine/", d(i).name]);
    data(i,:) = fread (fp, Inf, "float32");
    fclose (fp);
end

fp = fopen ("t-spine/t-spine.fann", "w");
fprintf (fp, "%d %d %d\n", length(sid), 256, 1);
for i = 1:length(sid)
    for j = 1:size(data,2)
        fprintf (fp, "%f ", data(i,j));
    end
    fprintf (fp, "\n%f\n", (sid(i) / 6) - 1);
                    %fprintf (fp, "\n%f\n", sid(i));
end
fclose (fp);
