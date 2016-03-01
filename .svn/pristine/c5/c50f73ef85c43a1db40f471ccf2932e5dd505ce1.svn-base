cd ([getenv('HOME'), '/Dropbox/autolabel']);
init_shogun

lung = [];
hn = [];

d = dir ("lung/*.raw");
for i = 1:length(d)
    fp = fopen (["lung/", d(i).name]);
    lung(i,:) = fread (fp, Inf, "float32");
    fclose (fp);
end

d = dir ("hn/*.raw");
for i = 1:length(d)
    fp = fopen (["hn/", d(i).name]);
    hn(i,:) = fread (fp, Inf, "float32");
    fclose (fp);
end

width = 2;
width = 20;
C = 1.0;
C = 100000000;

all_features = [lung; hn]';
all_features(all_features < -1000) = -1000;
all_features = [lung; hn]' / 1000 + 1.0;
all_labels = [ones(1, size(lung,1)), -ones(1, size(hn,1))];

data_len = size(all_features,2);

rp = randperm (data_len);
all_features = all_features(:,rp);
all_labels = all_labels(:,rp);
test_size = floor(data_len / 10);

train_features = all_features (:,1:data_len - test_size);
test_features = all_features (:,data_len-test_size+1:end);
train_labels = all_labels (1:data_len - test_size);
test_labels = all_labels (data_len-test_size+1:end);

train_f = RealFeatures (train_features);
test_f = RealFeatures (test_features);
kernel = GaussianKernel (train_f, train_f, width);
km = kernel.get_kernel_matrix();
train_l = Labels (train_labels);
test_l = Labels (test_labels);

svm = LibSVM (C, kernel, train_l);
%svm = SVMLight(C, kernel, train_l);
%svm = GMNPSVM(C, kernel, train_l);
svm.train();

output = svm.classify (test_f);
output_l = output.get_labels()
sign(output_l)
test_labels
std (output_l)
pm = PerformanceMeasures (test_l, output);
acc = pm.get_accuracy()



%roc = pm.get_auROC();
%fms = pm.get_fmeasure();
